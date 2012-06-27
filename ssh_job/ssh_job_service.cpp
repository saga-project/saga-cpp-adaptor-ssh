//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// system includes
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>


// stl includes
#include <vector>
#include <fstream>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/config.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/utils.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/attribute.hpp>

// saga package includes
#include <saga/saga/packages/job/adaptors/job.hpp>
#include <saga/saga/packages/job/adaptors/job_self.hpp>

// adaptor includes
#include "ssh_job_service.hpp"


////////////////////////////////////////////////////////////////////////
namespace ssh_job
{
  // constructor
  job_service_cpi_impl::job_service_cpi_impl (proxy                * p, 
                                              cpi_info const       & info,
                                              saga::ini::ini const & glob_ini, 
                                              saga::ini::ini const & adap_ini,
                                              TR1::shared_ptr <saga::adaptor> adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags)
    , s_       (p->get_session ()) // empty session
    , use_gsi_ (false)
      // Create a local job adaptor to spawn off ssh commands at will.  
      // If that throws, we simply pass on the exception.
  {
    adaptor_data  adata (this);
    instance_data idata (this);

    // create local job service which handles all job submissions.  This
    // may throw.
    js_ = saga::job::service ("fork://localhost");


    ini_ = adap_ini.get_section ("preferences").get_entries ();

    check_ini_ ();


    rm_   = idata->rm_;
    host_ = rm_.get_host ();
    port_ = rm_.get_port ();
    path_ = rm_.get_path ();


    if ( ! path_.empty () &&
           path_ != "/"   )
    {
      SAGA_ADAPTOR_THROW ("Cannot handle path in ssh URLs", 
                          saga::BadParameter);
    }

    // ssh declines localhost - that is taken care of by local adaptor...
    if ( host_.empty () )
    {
      // we can succeed w/o hostname for now, but may fail later on actually
      // running the job.  We will then assume either localhost, or one of the
      // candidate_hosts - but those may be invalid of course.
    }

    if ( rm_.get_scheme () != "gsissh" &&
         rm_.get_scheme () != "ssh"    &&
         rm_.get_scheme () != "any"    &&
         rm_.get_scheme () != "" )
    {
      SAGA_ADAPTOR_THROW (std::string ("Adaptor only supports these schemas: 'gsissh://', 'ssh://', 'any://', none, but not ")
                          + rm_.get_scheme ().c_str (),
                          saga::adaptors::AdaptorDeclined);
    }


    if ( rm_.get_scheme () == "gsissh" )
    {
      use_gsi_ = true;
    }



    if ( port_ > 0 )
    {
      std::stringstream ss;
      ss << port_;
      std::string port_s = ss.str ();

      if ( use_gsi_ )
      {
        gsissh_opt_.push_back ("-p");
        gsissh_opt_.push_back (port_s);
      }
      else
      {
        ssh_opt_.push_back ("-p");
        ssh_opt_.push_back (port_s);
      }
    }


    SAGA_LOG_DEBUG (rm_.get_string ().c_str ())


    // FIXME: we do not mess with contexts for gsissh - that relies on the
    // default user proxy location right now
    if ( ! use_gsi_ )
    {
      // check if we have a context for ssh
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
        // FIXME: isn't a warning enough?  ssh may be configured ok out-of-bound.
        SAGA_LOG_WARN ("no ssh context found for session");
      }


      for ( unsigned int j = 0; j < ssh_contexts.size (); j++ )
      {
        // try that context
        ctx_ = ssh_contexts[j];

        // FIXME: check if attribs exist
        if ( !     ctx_.attribute_exists ("UserKey")  ||
             "" == ctx_.get_attribute    ("UserKey")  ||
             !     ctx_.attribute_exists ("UserCert") ||
             "" == ctx_.get_attribute    ("UserCert") )
        {
          // _need_ private and public key to be useful
          SAGA_LOG_DEBUG ("context  incomplete");
          break;
        }

        loc_ssh_key_priv_  = ctx_.get_attribute ("UserKey");
        loc_ssh_key_pub_   = ctx_.get_attribute ("UserCert");

        struct passwd * p = ::getpwuid (::getuid ());
        if ( p == NULL )
        {
          local_user_ = "root";
        }
        local_user_ = p->pw_name;
        

        if ( ctx_.attribute_exists ("UserKey") )
        {
          user_ = ctx_.get_attribute ("UserID");
        } 
        else
        {
          user_ = local_user_;
        }


        // the URL may actually have a userid fixed
        if ( "" != rm_.get_userinfo () )
        {
          user_ = rm_.get_userinfo ();
        }


        // determine additional vars used for the environement of started jobs
        // FIXME: the saga jobid leads to invalid command lines and file names
        // try
        // {
        //   saga::job::job self = js_.get_self ();
        //   parent_id_ = self.get_job_id ();
        // }
        // catch ( ... )
        {
          // did not get jobid - invent one
          std::stringstream ss;
          ss << "[saga_parent_id:" << ::getpid () << "]";
          parent_id_ = ss.str ();
        }

        rem_ssh_key_pub_  = std::string ("/tmp/saga_") + parent_id_ + "_ssh.pub";
        rem_ssh_key_priv_ = std::string ("/tmp/saga_") + parent_id_ + "_ssh";


        // prepare the remote host
        if ( ini_["ssh_test_remote"] == "yes"  ||
             ini_["ssh_test_remote"] == "true" )
        {
          // we don't test if host is not known
          if ( ! host_.empty () )
          {
            SAGA_LOG_DEBUG (" running ssh test");

            saga::adaptors::utils::process proc;

            proc.set_cmd  (ssh_bin_);
            proc.set_args (ssh_opt_);

            // FIXME: ensure that context is complete
            proc.add_args ("-i", loc_ssh_key_priv_);
            proc.add_arg  (      user_ + "@" + host_);
            proc.add_arg  ("true");

            (void) proc.run_sync ();

            if ( ! proc.done () )
            {
              // SAGA_ADAPTOR_THROW ("Could not run a test ssh command", saga::NoSuccess);
              std::stringstream ss;
              ss << "Cannot execute jobs on remote host (" << proc.get_err_s () << ")";
              SAGA_ADAPTOR_THROW (ss.str (), saga::NoSuccess);
            }
          }
        }

        // we keep this context as valid for the host.
        // s_.add_context (ctx_);

        // we are done - no exception 'til now!
        return;
      }

    } 
    else // ! use_gsi_
    {
      // prepare the remote host

      if ( ini_["ssh_test_remote"] == "yes"  ||
           ini_["ssh_test_remote"] == "true" )
      {
        // we don't test if host is not known
        if ( ! host_.empty () )
        {
          SAGA_LOG_DEBUG (" running ssh test");

          saga::adaptors::utils::process proc;
          
          proc.set_cmd  (gsissh_bin_);
          proc.set_args (gsissh_opt_);

          proc.add_arg  (host_);
          proc.add_arg  ("true");

          (void) proc.run_sync ();

          if ( ! proc.done () )
          {
            // SAGA_ADAPTOR_THROW ("Could not run a test ssh command", saga::NoSuccess);
            std::stringstream ss;
            ss << "Cannot execute jobs on remote host (" << proc.get_err_s () << ")";
            SAGA_ADAPTOR_THROW (ss.str (), saga::NoSuccess);
          }
        }
      }

      // we are done - no exception 'til now!
      return;
    }



    // no context was ok for ssh, or preparation failed - flag error
    // FIXME: throw above when error occurs, with better error message
    SAGA_ADAPTOR_THROW ("Could not connect to remote host - missing context?", saga::NoSuccess);
  }

  // destructor
  job_service_cpi_impl::~job_service_cpi_impl (void)
  {
  }

  //////////////////////////////////////////////////////////////////////
  // SAGA API functions
  void job_service_cpi_impl::sync_create_job (saga::job::job         & ret, 
                                              saga::job::description   jd)
  {
    SAGA_LOG_DEBUG ("SSH Create Job");
    SAGA_LOG_DEBUG (rm_.get_string ().c_str ());

    std::vector <std::string> new_env;
    if ( jd.attribute_exists (saga::job::attributes::description_environment) )
    {
      new_env = jd.get_vector_attribute (saga::job::attributes::description_environment);
    }

    for ( unsigned int i = 0; i < env_.size (); i++ )
    {
      new_env.push_back (env_[i]);
    }

    jd.set_vector_attribute (saga::job::attributes::description_environment, new_env);

    // create the job with the 'fixed' job description
    saga::job::job job = saga::adaptors::job (rm_, jd, s_);
    ret = job;
  }

  void job_service_cpi_impl::sync_run_job (saga::job::job     & ret, 
                                           std::string          cmd, 
                                           std::string          host, 
                                           saga::job::ostream & in, 
                                           saga::job::istream & out, 
                                           saga::job::istream & err)
  {
    // we rely on the package fallback
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_service_cpi_impl::sync_list (std::vector <std::string> & ret)
  {
    std::vector <std::string> tmp = js_.list ();

    adaptor_data adata (this);

    for ( unsigned int i = 0; i < tmp.size (); i++ )
    {
      // FIXME: we should grep for ssh jobs

      // translate jobids from '[fork]-...' to '[ssh]-...' 
      ret.push_back (adata->translate_jobid (tmp[i]));
    }
  }


  void job_service_cpi_impl::sync_get_job (saga::job::job & ret,
                                           std::string      jobid)
  {
    instance_data idata (this);

    // we do *not* translate the job id - that will be taken care of by the job
    // ctor (using the init_from_jobid path)
    ret = saga::adaptors::job (idata->rm_,
                               jobid,
                               proxy_->get_session ());
  }
  


  void job_service_cpi_impl::sync_get_self (saga::job::self & ret)
  {
    // the ssh adaptor can never implement job::self
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  void job_service_cpi_impl::dump_context_ (saga::context c)
  {
    std::cout << " --------------------------------------\n";

    std::vector <std::string> attribs = c.list_attributes ();

    for ( unsigned int i = 0; i < attribs.size (); i++ )
    {
      std::string key = attribs[i];
      std::string val;

      if ( c.attribute_is_vector (key) )
      {
        std::vector <std::string> vals = c.get_vector_attribute (key);
        for ( unsigned int j = 0; j < vals.size (); j++ )
        {
          val += vals[j] + " ";
        }
      }
      else
      {
        val = c.get_attribute (key);
      }

      std::cout << key << " \t: " << val << std::endl;
    }

    std::cout << " --------------------------------------\n";
  }

  void job_service_cpi_impl::check_ini_ (void)
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


    // NOTE: ssh_spread_keys is disabled - see trac
    // // set sensible default options
    // if ( ini_.find ("ssh_spread_keys") == ini_.end () )
    // {
    //   ini_["ssh_spread_keys"] = "false";
    // }

    if ( ini_.find ("ssh_test_remote") == ini_.end () )
    {
      ini_["ssh_test_remote"] = "true";
    }
  }

} // namespace ssh_job
////////////////////////////////////////////////////////////////////////

