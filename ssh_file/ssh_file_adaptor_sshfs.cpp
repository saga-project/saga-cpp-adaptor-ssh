//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/adaptors/utils/utils.hpp>
#include <saga/impl/exception.hpp>

#include "ssh_file_adaptor_sshfs.hpp"

namespace ssh_filesystem_adaptor
{
  sshfs::sshfs (const ini_t         & i,
                const saga::session & s, 
                const saga::url     & u) 
    : s_       (s)
    , url_     (u)
    , ini_     (i)
    , id_      ("")
    , mounted_ (false)
  {
    SAGA_LOG_DEBUG (u.get_string ().c_str ());

    // we *only* accept ssh URLs: any:// must be translated into ssh:// by the
    // caller!
    if ( url_.get_scheme () != "ssh" )
    {
      SAGA_LOG_ALWAYS ("sshfs can not handle url:");
      SAGA_LOG_ALWAYS (u.get_string ().c_str ());

      std::stringstream ss;
      ss << "BadParameter: Cannot handle URL scheme " << u.get_scheme ()
        << " - can only handle schema 'ssh'."         << std::endl;
      SAGA_ADAPTOR_THROW_NO_CONTEXT (ss.str ().c_str (), saga::adaptors::AdaptorDeclined);
    }


    host_ = url_.get_host ();
    port_ = url_.get_port ();

    // default host/port
    if ( host_ == "" ) { host_ = "localhost"; } 
    if ( port_ == -1 ) { port_ = 22;          }

    std::stringstream ss;
    ss << port_;
    port_s_ = ss.str ();

    // check if ini has mandatory entries
    // FIXME: this should be done _once_ in the adaptor
    check_ini_ ();


    // attempt to mount the file system via sshfs

    // get an ssh context from the session which allows to access the host of
    // the given URL
    std::vector <saga::context> contexts = s_.list_contexts ();

    // error stub
    std::string err ("Cannot mount file system with sshfs: \n");

    for ( unsigned int i = 0; i < contexts.size (); i++ )
    {
      if ( contexts[i].get_attribute (saga::attributes::context_type) == "ssh" )
      {
        ctx_ = contexts[i];

        if ( ctx_.attribute_exists (saga::attributes::context_userid)  &&
             ctx_.attribute_exists (saga::attributes::context_userkey) )
        {
          user_     = ctx_.get_attribute (saga::attributes::context_userid);
          userkey_  = ctx_.get_attribute (saga::attributes::context_userkey);
          identity_ = "IdentityFile=" + userkey_;

          // the URL may actually have a userid fixed
          if ( "" != url_.get_userinfo () )
          {
            user_ = url_.get_userinfo ();
          }


          // default user
          if ( user_ == "" ) { user_ = "root"; }

          // get id 
          id_    = user_ + "@" + host_ + ":" + port_s_;

          // try to mount with that context
          mount ();

          if ( mounted_ )
          {
            return;
          }

          err += error_ + "\n";
        }
      }
    }

    // no mount - flag error
    SAGA_ADAPTOR_THROW_NO_CONTEXT (err.c_str (), saga::NoSuccess);
  }


  sshfs::~sshfs (void) 
  {
    if ( ini_.find ("keepalive") != ini_.end () &&
         ( ini_["keepalive"] == "yes"  ||
           ini_["keepalive"] == "true" )        )
    {
      SAGA_LOG_WARN ("keeping SSHFS filesystem mounted at ");
      SAGA_LOG_WARN (mount_.c_str ());
    }
    else
    {
      SAGA_LOG_WARN ("umounting SSHFS filesystem at ");
      SAGA_LOG_WARN (mount_.c_str ());

      umount ();
    }
  }


  // mount a sshfs file system, and register it
  void sshfs::mount (void)
  {
    if ( mounted_ )
    {
      return;
    }

    // get local mounty point (no '//' allowed)
    mount_ = sshfs_mnt_ + "/" + id_;

    while ( mount_.find ("//") != std::string::npos )
    {
      int n = mount_.find ("//");
      mount_.replace (n, 2, "/");
    }


    // check if the fs is already mounted
    {
      saga::adaptors::utils::process proc ("mount");

      proc.run_sync ();

      if ( proc.fail () )
      { 
        SAGA_ADAPTOR_THROW_NO_CONTEXT ("cannot check if fs is mounted", saga::NoSuccess);
      }

      std::vector <std::string> lines = saga::adaptors::utils::grep (mount_,
                                                                     proc.get_out_v ());

      // should find zero or one matching line
      switch ( lines.size () )
      {
        case 0:
          mounted_ = false;
          break;
        case 1:
          mounted_ = true;
          break;
        default:
          SAGA_ADAPTOR_THROW_NO_CONTEXT ("cannot check if fs is mounted", saga::NoSuccess);
          break;
      }

      if ( mounted_ )
      {
        SAGA_LOG_INFO ("SSHFS already mounted");
        return;
      }
    }


    // create local mount point
    try 
    {
      saga::filesystem::directory root ("file://localhost/");

      if ( ! root.exists (mount_) ||
           ! root.is_dir (mount_) )
      {
        root.open_dir (mount_, 
                       saga::filesystem::Create        |
                       saga::filesystem::CreateParents |
                       saga::filesystem::Overwrite     );
      }
    }
    catch ( const saga::exception & e )
    {
      SAGA_LOG_ERROR (e.what ());
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("cannot create local mount point", saga::NoSuccess);
    }


    // run mount process
    {
      saga::adaptors::utils::process proc (sshfs_bin_);

      proc.set_args (sshfs_opt_);                  // default options
      proc.add_args ("-p", port_s_);               // ssh port to use

      if ( ! identity_.empty () )
      {
        proc.add_args ("-o", identity_);           // ssh key to use
      }

      if ( ! user_.empty () )
      {
        proc.add_arg (user_ + "@" + host_ + ":/"); // remote user + mount point
      }
      else
      {
        proc.add_arg (host_ + ":/");               // remote mount point
      }
      proc.add_arg  (mount_);                      // local mount point
      proc.run_sync ();

      if ( proc.done () )
      { 
        mounted_ = true;
        SAGA_LOG_DEBUG ("new sshfs mounted");
      }
      else
      {
        error_ = proc.get_err_s ();
        mounted_ = false;
        SAGA_LOG_ALWAYS ("sshfs mount failed");
        SAGA_LOG_ALWAYS (error_.c_str ());
      }
    }

    if ( mounted_ )
    {
      // create and set id
      if ( id_.empty () )
      {
        if ( ! user_.empty () )
        {
          id_ += user_ + "@";
        }

        id_ += host_ + ":" + port_s_;
      }
    }

  }

  void sshfs::umount (void)
  {
    // FIXME: is not being called yet??
    saga::adaptors::utils::process proc ("umount");

    proc.add_arg (mount_);
    proc.run_sync ();

    if ( proc.fail () )
    {
      // we can't really do much about that error.
      // Lets at least warn the user!
      SAGA_LOG_CRITICAL ("Could not umount SSHFS filesystem at ");
      SAGA_LOG_CRITICAL (mount_.c_str ());
    }
  }


  saga::url sshfs::translate (const saga::url & u)
  {
    if ( ! mounted_ )
      return u;

    // others need to point to this instance's remote system to be translateable
    if ( ( u.get_host   () != host_ )  ||
         ( u.get_port   () != port_ &&
           u.get_port   () != -1    )  ||
         ( u.get_scheme () != "ssh" &&
           u.get_scheme () != "any" )  )
    {
      // nothing to do here
      return u;
    }
    else
    {
      return saga::url ("file://localhost/" + mount_ + u.get_path ());
    }

  }


  saga::url sshfs::translate_back (const saga::url & u)
  {
    saga::url ret;

    // only local names have any chance of getting translated
    if ( ( u.get_host   () != "localhost" &&
           u.get_host   () != ""          )   || 

         ( u.get_port   () != -1          )   ||

         ( u.get_scheme () != "file"      &&
           u.get_scheme () != "any"       &&
           u.get_scheme () != ""          )   )
    {
      // nothing to do here
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("cannot translate URL back",
                                     saga::NoSuccess);
    }

    std::string path = saga::url::unescape (u.get_path ());

    // check if path points into the mounted name space
    if ( path.find (mount_) != 0 )
    {
      // nop - nothing to do
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("Cannot translate URL back",
                                     saga::NoSuccess);
    }

    path.erase (0, mount_.size ());

    ret = url_;           // original name space
    ret.set_path (path);  // backtranslated path component

    return ret;
  }


  std::string sshfs::get_id (void)
  {
    return (id_);
  }


  void sshfs::check_ini_ (void)
  {
    // check if ini has required entries
    if ( ini_.find ("ssh_bin") == ini_.end () ||
         ini_["ssh_bin"]       == ""          )
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("need path to ssh_bin specified in the SAGA ini",
                                     saga::NoSuccess);
    }

    if ( ini_.find ("sshfs_bin") == ini_.end () ||
         ini_["sshfs_bin"]       == ""          )
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("need path to sshfs specified in the SAGA ini",
                                     saga::NoSuccess);
    }

    // check if ini has required entries
    if ( ini_.find ("sshfs_mnt") == ini_.end () ||
         ini_["sshfs_mnt"]       == ""          )
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("need sshfs_mnt specified in the SAGA ini",
                                     saga::NoSuccess);
    }

    // set default opts (none)
    if ( ini_.find ("ssh_opt") == ini_.end () )
    {
      ini_["ssh_opt"] = "";
    }

    if ( ini_.find ("sshfs_opt") == ini_.end () )
    {
      // set default opts (none)
      ini_["sshfs_opt"] = "";
    }


    ssh_bin_   = ini_["ssh_bin"];
    ssh_opt_   = saga::adaptors::utils::split (ini_["ssh_opt"], ' ');
    sshfs_bin_ = ini_["sshfs_bin"];
    sshfs_opt_ = saga::adaptors::utils::split (ini_["sshfs_opt"], ' ');
    sshfs_mnt_ = ini_["sshfs_mnt"];
  }

} // namespace ssh_filesystem_adaptor

