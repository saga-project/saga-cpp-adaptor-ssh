//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "sshfs_file_adaptor_file.hpp"

namespace sshfs_filesystem_adaptor
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_permissions_allow (saga::impl::void_t & ret,
                                              std::string          id, 
                                              int                  perm, 
                                              int                  flags)
  {
    SSH_ADAPTOR_TRY
      f_.permissions_allow (id, perm, flags);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_permissions_deny  (saga::impl::void_t & ret, 
                                              std::string          id, 
                                              int                  perm, 
                                              int                  flags)
  {
    SSH_ADAPTOR_TRY
      f_.permissions_deny (id, perm, flags);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_permissions_check (bool        & ret, 
                                              std::string   id, 
                                              int           perm)
  {
    SSH_ADAPTOR_TRY
      f_.permissions_check (id, perm);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_get_owner (std::string & out)
  {
    SSH_ADAPTOR_TRY
      out = f_.get_owner ();
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_get_group (std::string & out)
  {
    SSH_ADAPTOR_TRY
      out = f_.get_group ();
    SSH_ADAPTOR_CATCH;
  }

} // namespace

