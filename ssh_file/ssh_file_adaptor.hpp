//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_SSH_FILE_ADAPTOR_HPP
#define ADAPTORS_SSH_FILE_ADAPTOR_HPP

#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/impl/engine/proxy.hpp>
#include <saga/impl/filesystem.hpp>
#include <saga/saga/filesystem.hpp>

#include "ssh_file_adaptor_dir.hpp"
#include "ssh_file_adaptor_file.hpp"
#include "ssh_file_adaptor_sshfs.hpp"

////////////////////////////////////////////////////////////////////////////
#define SSH_ADAPTOR_TRY                                                    \
  try {                                                                    \
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
#define SSH_ADAPTOR_CATCH                                                  \
  } catch ( const saga::exception & e )                                    \
  {                                                                        \
    std::cout << "exception : " << e.what () << std::endl;                 \
    SSH_ADAPTOR_RETHROW (e, "operation failed on mounted file system",     \
                        saga::NoSuccess);                                  \
  }                                                                        \
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
#define SSH_ADAPTOR_RETHROW(e, msg, type)                                  \
  SAGA_ADAPTOR_THROW (std::string (msg) + ": " + e.get_message (),         \
                      type);                                               \
////////////////////////////////////////////////////////////////////////////


namespace ssh_filesystem_adaptor
{
  typedef      saga::adaptors::v1_0::file_cpi_instance_data file_idata_type;
  typedef      saga::adaptors::instance_data <file_idata_type> file_idata_t;

  typedef      saga::adaptors::v1_0::directory_cpi_instance_data dir_idata_type;
  typedef      saga::adaptors::instance_data <dir_idata_type> dir_idata_t;


  //////////////////////////////////////////////////////////////////////
  //
  // the file adaptor class
  //
  class filesystem_adaptor : public saga::adaptor
  {
    private:
      // list of mounted sshfs file system
      std::map <std::string, TR1::shared_ptr <sshfs> > mounted_;
      std::map <std::string, int                     > tested__;

      // cache of ini section
      ini_t ini_;

    public:
      typedef saga::impl::v1_0::op_info         op_info;  
      typedef saga::impl::v1_0::cpi_info        cpi_info;
      typedef saga::impl::v1_0::preference_type preference_type;

      filesystem_adaptor (void);

      ~filesystem_adaptor (void);

      std::string get_name (void) const
      {
        return BOOST_PP_STRINGIZE (SAGA_ADAPTOR_NAME);
      }

      /**
       * This functions registers the adaptor with the factory
       *
       * @param factory the factory where the adaptor registers
       *        its maker function and description table
       */
      saga::impl::adaptor_selector::adaptor_info_list_type 
        adaptor_register (saga::impl::session * s);

      // adaptor initialization, mainly for caching the ini files
      bool init (saga::impl::session  * s,
                 saga::ini::ini const & glob_ini,
                 saga::ini::ini const & adap_ini);


      // obtain an sshfs mount point for some URL, and some related helpers
      saga::session           strip_session      (const saga::session & s);
      TR1::shared_ptr <sshfs> get_sshfs          (const saga::session & s, 
                                                  const saga::url     & u);
      TR1::shared_ptr <sshfs> mount_sshfs        (const saga::session & s, 
                                                  const saga::url     & u);
      std::string             get_sshfs_id       (const saga::url     & u);
      saga::url               translate          (const saga::session & s, 
                                                  const saga::url     & u);
      saga::url               try_translate      (const saga::session & s, 
                                                  const saga::url     & u);
      saga::url               translate_back     (const saga::session & s, 
                                                  const saga::url     & u);
      saga::url               try_translate_back (const saga::session & s, 
                                                  const saga::url     & u);
      bool                    is_absolute        (const saga::url     & u);
      saga::url               make_absolute      (const saga::url     & base, 
                                                  const saga::url     & u);
      void                    dump_session       (const std::string   & msg, 
                                                  const saga::session & s);
  };

} // namespace ssh_filesystem_adaptor

#endif // ADAPTORS_SSH_FILE_ADAPTOR_HPP

