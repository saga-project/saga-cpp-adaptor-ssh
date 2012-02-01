//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_SSHFS_FILE_ADAPTOR_SSHFS_HPP
#define ADAPTORS_SSHFS_FILE_ADAPTOR_SSHFS_HPP

#include <map>

#include <saga/saga.hpp>

namespace sshfs_filesystem_adaptor 
{
  class sshfs;

  typedef std::map <std::string, std::string> ini_t;
  typedef boost::shared_ptr <sshfs>           sshfs_t;

  // a structure pointing to an sshfs mounted file systems.  Shared_ptr's to
  // this class are used for life time management of these mounts: each file or
  // directory which is open and points into that file system should keep such
  // a shared_ptr.
  class sshfs
  {
    private:
      saga::session               s_;           // session used for mount
      saga::context               ctx_;         // context used for mount
      saga::url                   url_;         // URL which triggered the mount

      std::string                 user_;        // remote user id
      std::string                 host_;        // remote host
      int                         port_;        // ssh port on remote host
      std::string                 port_s_;      // ssh port on remote host (string)
      std::string                 userkey_;     // ssh key
      std::string                 identity_;    // ssh key option

      ini_t                       ini_;         // preferences

      std::string                 id_;          // unique id identifying the mount
      std::string                 mount_;       // local mountpoint
      bool                        mounted_;     // flag success

      std::string                 ssh_bin_;     // path to ssh
      std::vector <std::string>   ssh_opt_;     // options to ssh
      std::string                 sshfs_bin_;   // path to sshfs
      std::vector <std::string>   sshfs_opt_;   // options to sshfs
      std::string                 sshfs_mnt_;   // mountpoint base

      std::string                 error_;       // keep last error message

      // helper functions
      void check_ini_ (void);


    public:
      sshfs  (const ini_t         & i,
              const saga::session & s, 
              const saga::url     & u);
      ~sshfs (void);

      void        mount          (void);
      void        umount         (void);
      std::string get_error      (void) { return error_; error_ = ""; }

      std::string get_id         (void);
      saga::url   translate      (const saga::url & u);
      saga::url   translate_back (const saga::url & u);
  };
}

#endif // ADAPTORS_SSHFS_FILE_ADAPTOR_SSHFS_HPP

