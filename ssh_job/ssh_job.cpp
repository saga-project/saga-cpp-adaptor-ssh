//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// system includes
#include <pwd.h>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga adaptor icnludes
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/attribute.hpp>
#include <saga/saga/adaptors/file_transfer_spec.hpp>

// saga engine includes
#include <saga/impl/config.hpp>
#include <saga/impl/exception_list.hpp>

// saga package includes
#include <saga/saga/packages/job/adaptors/job_self.hpp>
#include <saga/saga/packages/job/job_description.hpp>

// adaptor includes
#include "ssh_job.hpp"
#include "ssh_job_istream.hpp"
#include "ssh_job_ostream.hpp"


////////////////////////////////////////////////////////////////////////
namespace ssh_job
{

  // constructor
  //
  // note that we can never be job::self - that is usually handled by the local
  // adaptor.
  job_cpi_impl::job_cpi_impl (proxy                           * p, 
                              cpi_info const                  & info,
                              saga::ini::ini const            & glob_ini, 
                              saga::ini::ini const            & adap_ini,
                              TR1::shared_ptr <saga::adaptor>   adaptor)
    : base_cpi  (p, info, adaptor, cpi::Noflags)
    , use_gsi_  (false)
  {
    instance_data idata (this);
    adaptor_data  adata (this);

    ini_ = adap_ini.get_section ("preferences").get_entries ();
    
    check_ini_ ();

    rm_  = idata->rm_;

    SAGA_LOG_DEBUG ("SSH Job c'tor");
    SAGA_LOG_DEBUG (rm_.get_string ().c_str ());

    host_ = rm_.get_host ();
    port_ = rm_.get_port ();
    path_ = rm_.get_path ();

    if ( ! path_.empty () &&
           path_ != "/"   )
    {
      SAGA_ADAPTOR_THROW ("Cannot handle path in gsissh or ssh URLs", 
                          saga::adaptors::AdaptorDeclined);
    }

    if (   rm_.get_scheme () != "gsissh" &&
           rm_.get_scheme () != "ssh"    &&
           rm_.get_scheme () != "any"    &&
         ! rm_.get_scheme ().empty () )
    {
      SAGA_ADAPTOR_THROW (std::string ("Adaptor only supports these schemas: 'ssh://', 'gsissh://', 'any://', "
                                       "none, but not ") + rm_.get_scheme ().c_str (),
                          saga::adaptors::AdaptorDeclined);
    }

    if ( rm_.get_scheme () == "gsissh" )
    {
      use_gsi_ = true;
    }

    // ssh declines localhost - that is taken care of by local adaptor...
    if ( host_.empty () )
    {
      // we can succeed w/o hostname for now, but may fail later on actually
      // running the job.  We will then assume either localhost, or one of the
      // candidate_hosts - but those may be invalid of course.
    }

    gsissh_opt_ = saga::adaptors::utils::split (ini_["gsissh_opt"], ' ');


    // FIXME: we do not mess with contexts for gsissh - that relies on the
    // default user proxy location right now
    if ( ! use_gsi_ )
    {
      ssh_opt_    = saga::adaptors::utils::split (ini_["ssh_opt"],    ' ');

      // trust the job service to give us a session with a valid context
      std::vector <saga::context> contexts = p->get_session ().list_contexts ();    
      std::vector <saga::context> ssh_contexts;

      for ( unsigned int i = 0; i < contexts.size (); i++ )
      {
        if ( contexts[i].attribute_exists (saga::attributes::context_type) &&
             contexts[i].get_attribute (saga::attributes::context_type) == "ssh" )
        {
          SAGA_LOG_DEBUG ("found ssh context");
          ssh_contexts.push_back (contexts[i]);
        }
      }
      
      if ( 0 == ssh_contexts.size () )
      {
        // ssh may be configured ok out-of-bound, so just a warning
        SAGA_LOG_WARN ("no ssh context found for session");
      }
      else
      {
        // we should try one context after the other, just as the job service
        // does.  For now, just use the first one available
        ctx_ = ssh_contexts[0];
      }


      // sanity check
      if ( ctx_.attribute_exists ("Type") &&
           ctx_.get_attribute    ("Type") == "ssh" )
      {
        if ( ! ctx_.attribute_exists ("UserKey") )
        {
          // _need_ key to be useful
          SAGA_ADAPTOR_THROW_NO_CONTEXT ("ssh context has no userkey",
                                         saga::adaptors::AdaptorDeclined);
        }

        key_  = ctx_.get_attribute ("UserKey");


        if ( ctx_.attribute_exists ("UserKey") )
        {
          user_ = ctx_.get_attribute ("UserID");
        } 
        else
        {
          struct passwd * p = ::getpwuid (::getuid ());
          if ( p == NULL )
          {
            user_ = "root";
          }
          else
          {
            user_ = p->pw_name;
          }
        }
      }
      else
      {
        struct passwd * p = ::getpwuid (::getuid ());
        if ( p == NULL )
        {
          user_ = "root";
        }
        else
        {
          user_ = p->pw_name;
        }
        
        key_ = "";
      }

      // the URL may actually have a userid fixed
      if ( "" != rm_.get_userinfo () )
      {
        user_ = rm_.get_userinfo ();
      }

    } // ! use_gsi_


    SAGA_LOG_DEBUG (rm_.get_string ().c_str ());


    // create local job service which handles all job submissions
    js_ = TR1::shared_ptr <saga::job::service> (new saga::job::service ("fork://localhost"));


    if ( idata->init_from_jobid_ )
    {
      // note that the jobid stored in the instance data is the job id from the
      // fork adaptor, so we do *not* need to translate it.

      // save jobid we want to connect to
      std::string old_id = idata->jobid_;
      std::string new_id = adata->translate_jobid (old_id);

      j_  = js_->get_job (new_id);

      // FIXME: should get hostname from jd
      id_ = adata->translate_jobid (j_.get_job_id (), "localhost");

      // restore job id (which got trsanslated to fork:// on the way
      idata->jobid_ = old_id;
    }
    else
    {
      // we don't init from jobid, thus we init from job description
      // check if we can create that job for the target host
      // FIXME: clone should not be needed according to spec
      old_jd_ = idata->jd_.clone ();

      j_ = create_local_job_ (host_);
    }
  }


  // destructor
  job_cpi_impl::~job_cpi_impl (void)
  {
  }


  //  SAGA API functions
  void job_cpi_impl::sync_get_state (saga::job::state & ret)
  {
    ret = j_.get_state ();
  }

  void job_cpi_impl::sync_get_description (saga::job::description & ret)
  {
    // simply return the description we originally received
    // FIXME: need to re-create the description for reconnected jobs, i.e. need
    // to remove the ssh specific command/arguments etc.
    // FIXME: clone should not be needed according to spec
    ret = old_jd_.clone ();
    
  }

  void job_cpi_impl::sync_get_job_id (std::string & ret)
  {
    adaptor_data  adata (this);
    instance_data idata (this);

    // translate local jobid into ssh jobid, 
    // FIXME: host should actually be parsed from the ssh command line, for
    // reconnecting jobs!
    ret = adata->translate_jobid (j_.get_job_id (), host_);
  }

  // access streams for communication with the child
  void job_cpi_impl::sync_get_stdin (saga::job::ostream & ret)
  {
    ret = j_.get_stdin ();
  }

  void job_cpi_impl::sync_get_stdout (saga::job::istream & ret)
  {
    ret = j_.get_stdout ();
  }

  void job_cpi_impl::sync_get_stderr (saga::job::istream & ret)
  {
    ret = j_.get_stderr ();
  }

  void job_cpi_impl::sync_checkpoint (saga::impl::void_t & ret)
  {
    j_.checkpoint ();
  }

  void job_cpi_impl::sync_migrate (saga::impl::void_t   & ret, 
                                   saga::job::description jd)
  {
    SAGA_ADAPTOR_THROW_NO_CONTEXT ("migrate is not supported by the ssh backend",
                                    saga::NotImplemented);
    // FIXME: jd should be translated just as in the c'tor
    // j_.migrate (jd);
  }

  void job_cpi_impl::sync_signal (saga::impl::void_t & ret, 
                                  int                  signal)
  {
    SAGA_ADAPTOR_THROW_NO_CONTEXT ("signal is not supported by the ssh backend",
                                    saga::NotImplemented);
  }


  //  suspend the child process 
  void job_cpi_impl::sync_suspend (saga::impl::void_t & ret)
  {
    SAGA_ADAPTOR_THROW_NO_CONTEXT ("suspend is not supported by the ssh backend",
                                    saga::NotImplemented);
  }

  //  suspend the child process 
  void job_cpi_impl::sync_resume (saga::impl::void_t & ret)
  {
    SAGA_ADAPTOR_THROW_NO_CONTEXT ("resume is not supported by the ssh backend",
                                    saga::NotImplemented);
  }


  //////////////////////////////////////////////////////////////////////
  // inherited from the task interface
  void job_cpi_impl::sync_run (saga::impl::void_t & ret)
  {
    // if no target host is specified, yet, we cycle over candidate hosts, or
    // fall back to localhost
    std::vector <std::string> target_hosts;

    if ( old_jd_.attribute_exists (saga::job::attributes::description_candidate_hosts) )
    {
      target_hosts = old_jd_.get_vector_attribute (saga::job::attributes::description_candidate_hosts);
    }
    else
    {
      target_hosts.push_back (host_);

      if ( host_ != "localhost" )
      {
        target_hosts.push_back ("localhost");
      }
    }

    // we keep the first exception we find, and rethrow it when everything else
    // fails
    // FIXME: should actually create an exception list/stack
    saga::exception first_exception ("Job execution failed: invalid target host", 
                                     saga::BadParameter);
    bool first   = true;
    bool success = false;

    for ( unsigned int i = 0; ! success && i < target_hosts.size (); i++ )
    {
      try
      {
        // try to create job for target host, if we don't have that one, yet
        if ( host_ != target_hosts[i] )
        {
          j_ = create_local_job_ (target_hosts[i]);
        }

        // we do not check for the executable anymore -- that way, we can run
        // full sh command lines, like 'cd /tmp && pwd'
        //
        // // check if executable exists
        // if ( ini_["ssh_test_remote"] == "yes"  ||
        //      ini_["ssh_test_remote"] == "true" )
        // {
        //   SAGA_LOG_DEBUG (" check if exe exists");
        // 
        //   saga::adaptors::utils::process proc;
        // 
        //   proc.set_cmd  (ssh_bin_);
        //   proc.set_args (ssh_args_);
        // 
        //   proc.add_arg  ("which");
        //   proc.add_arg  (old_exe_);
        // 
        //   (void) proc.run_sync ();
        // 
        //   if ( ! proc.done () )
        //   {
        //     std::stringstream ss;
        //     ss << "Cannot find executable " << old_exe_ << " on remote host:" << proc.get_err_s ();
        //     SAGA_ADAPTOR_THROW (ss.str (), saga::BadParameter);
        //   }
        // }

        j_.run ();

        success = true;
      }
      catch ( saga::exception & e )
      {
        if ( first )
        {
          first_exception = e;
          first = false;
        }
      }
    } // for all canidate hosts

    if ( ! success )
    {
      throw first_exception;
    }
  }

  void job_cpi_impl::sync_cancel (saga::impl::void_t & ret, 
                                  double timeout)
  {
    // FIXME: this is a tricky one: at the moment, the signal goes to the local
    // ssh process, not to the remote process!  Not easy to fix...  
    // OTOH, killing the local ssh process *does* kill the remote process, too
    // - just not the nice way, and not always...
    j_.cancel (timeout);
  }

  //  wait for the child process to terminate
  void job_cpi_impl::sync_wait (bool   & ret, 
                                double   timeout)
  {
    j_.wait (timeout);

    if ( saga::job::Done     == j_.get_state () ||
         saga::job::Failed   == j_.get_state () ||
         saga::job::Canceled == j_.get_state () )
    {
      ret = true;

      saga::adaptors::attribute attr (this);
      attr.set_attribute (saga::job::attributes::exitcode,
                          j_.get_attribute (saga::job::attributes::exitcode));
    }
  }

  void job_cpi_impl::check_ini_ (void)
  {
    // check if ini has required entries
    if ( ( ini_.find ("gsissh_bin") == ini_.end () ||
           ini_["gsissh_bin"]       == ""          )  
         &&
         ( ini_.find ("ssh_bin")    == ini_.end () ||
           ini_["ssh_bin"]          == ""          )  )
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("need path to gsissh_bin or ssh_bin in the SAGA ini",
                                     saga::NoSuccess);
    }

    // set default opts (none)
    if ( ini_.find ("gsissh_opt") == ini_.end () )
    {
      ini_["gsissh_opt"] = "";
    }

    gsissh_bin_   = ini_["gsissh_bin"];
    gsissh_opt_   = saga::adaptors::utils::split (ini_["gsissh_opt"], ' ');


    // set default opts (none)
    if ( ini_.find ("ssh_opt") == ini_.end () )
    {
      ini_["ssh_opt"] = "";
    }

    ssh_bin_   = ini_["ssh_bin"];
    ssh_opt_   = saga::adaptors::utils::split (ini_["ssh_opt"], ' ');


    if ( ini_.find ("ssh_test_remote") == ini_.end () )
    {
      ini_["ssh_test_remote"] = "true";
    }
  }


  ////////////////////////////////////////////////////////////////////////////
  //
  // get job from local job adaptor for specific job description
  //
  saga::job::job job_cpi_impl::create_local_job_ (std::string host)
  {
    if ( host.empty () )
    {
      // fall back...
      host = "localhost";
    }


    // we simply add the ssh command in front of the job cmd line, and reuse the
    // jd
    // FIXME: clone should not be needed according to spec
    new_jd_ = old_jd_.clone (); // save and fix for ssh

    // we need to unset some jd attribs which are incorrectly eval'ed by the
    // local job adaptor
    if ( new_jd_.attribute_exists (saga::job::attributes::description_input) )
    {
      new_jd_.set_attribute (saga::job::attributes::description_input, "");
    }

    if ( new_jd_.attribute_exists (saga::job::attributes::description_output) )
    {
      new_jd_.set_attribute (saga::job::attributes::description_output, "");
    }

    if ( new_jd_.attribute_exists (saga::job::attributes::description_error) )
    {
      new_jd_.set_attribute (saga::job::attributes::description_error, "");
    }

    // save old exe and args
    std::vector <std::string> old_args;

    if ( !  old_jd_.attribute_exists (saga::job::attributes::description_executable) )
    {
      SAGA_ADAPTOR_THROW ("job description incomplete: missing executable",
                           saga::BadParameter);
    }

    old_exe_ = old_jd_.get_attribute (saga::job::attributes::description_executable);

    if ( old_jd_.attribute_exists (saga::job::attributes::description_arguments) )
    {
      old_args = old_jd_.get_vector_attribute (saga::job::attributes::description_arguments);
    }

    // if the job is non-interactive, we need to add '-n' to the ssh command
    bool interactive = false;
    if ( old_jd_.attribute_exists (saga::job::attributes::description_interactive) )
    {
      if ( old_jd_.get_attribute (saga::job::attributes::description_interactive)
           == saga::attributes::common_true )
      {
        interactive = true;
      }
    }

    if ( old_jd_.attribute_exists (saga::job::attributes::description_input ) ||
         old_jd_.attribute_exists (saga::job::attributes::description_output) ||
         old_jd_.attribute_exists (saga::job::attributes::description_error ) )
    {
      interactive = true;
    }
    

    // create new args and exe, and set them in the new jd
    std::string               new_exe  = ssh_bin_;
    std::vector <std::string> new_args = ssh_opt_;

    if ( use_gsi_ )
    {
      new_exe  = gsissh_bin_;
      new_args = gsissh_opt_;
    }
    else
    {
      new_exe  = ssh_bin_;
      new_args = ssh_opt_;
    }


    std::vector <std::string> new_hosts;

    // suppress warnings
    new_args.push_back ("-q");

    // add ssh specific args
    if ( ! key_.empty () )
    {
      new_args.push_back ("-i");
      new_args.push_back (key_);
    }

    if ( ! interactive )
    {
      // adding '-t' twice is not an error!  See ssh manpage for 
      // details on '-n' and '-t'
      new_args.push_back ("-t");
      new_args.push_back ("-t");
      new_args.push_back ("-n");
    }

    if ( ! user_.empty () )
    {
      new_args.push_back (user_ + "@" + host);
    }
    else
    {
      new_args.push_back (host);
    }

    if ( port_ > 0 )
    {
      std::stringstream ss;
      ss << port_;

      new_args.push_back ("-p");
      new_args.push_back (ss.str ());
    }

    // save args so far for later reuse
    ssh_args_ = new_args;

    // we create a command as separate string, and give that as single
    // argument to ssh.  In fact, our remote command has always the form
    // "/bin/sh -c 'orginal --comand --with=args'"
    std::string cmd ("/bin/sh -c '");

    // if the js contains a working directory, we first execute a remote cd
    // to that directory, before running the job
    if ( old_jd_.attribute_exists (saga::job::attributes::description_working_directory) )
    {
      std::string cwd = old_jd_.get_attribute (saga::job::attributes::description_working_directory);

      // the new jd will usually not be able to handle the given WD
      new_jd_.set_attribute (saga::job::attributes::description_working_directory, "/tmp/");

      cmd += "cd " + cwd + " && ";
    }

    // add environment as ssh, via /usr/bin/env
    if ( old_jd_.attribute_exists (saga::job::attributes::description_environment) )
    {
      std::vector <std::string> env = old_jd_.get_vector_attribute 
                                        (saga::job::attributes::description_environment);

      if ( env.size () > 0 )
      {
        cmd += "/usr/bin/env";

        for ( unsigned int i = 0; i < env.size (); i++ )
        {
          cmd += " " + env[i];
        }
      }
    }


    // read old exe and args
    cmd += " " + old_exe_;

    for ( unsigned int i = 0; i < old_args.size (); i++ )
    {
      cmd += " " + old_args[i];
    }

    // add io indirection
    if ( interactive )
    {
      if ( old_jd_.attribute_exists (saga::job::attributes::description_output) )
      {
        cmd += " 1>" + old_jd_.get_attribute (saga::job::attributes::description_output);
      }

      if ( old_jd_.attribute_exists (saga::job::attributes::description_error ) )
      {
        cmd += " 2>" + old_jd_.get_attribute (saga::job::attributes::description_error);
      }

      if ( old_jd_.attribute_exists (saga::job::attributes::description_input ) )
      {
        cmd += " <" + old_jd_.get_attribute (saga::job::attributes::description_input);
      }
    }

    // close command off with '
    cmd += "'";

    // add remote ssh command as argument to the job we want to start locally
    new_args.push_back (cmd);

    // we only want to run on localhost
    new_hosts.push_back ("localhost");

    // fill jd with new settings
    new_jd_.set_attribute        (saga::job::attributes::description_executable,       new_exe);
    new_jd_.set_vector_attribute (saga::job::attributes::description_arguments,        new_args);
    new_jd_.set_vector_attribute (saga::job::attributes::description_candidate_hosts,  new_hosts);

    // create the job with the altered jd
    return js_->create_job (new_jd_);
  }

} // namespace ssh_job
////////////////////////////////////////////////////////////////////////

