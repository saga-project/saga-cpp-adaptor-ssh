//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "sshfs_file_adaptor_dir.hpp"

namespace sshfs_filesystem_adaptor
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_permissions_allow (saga::impl::void_t & ret, 
                                             saga::url            tgt, 
                                             std::string          id, 
                                             int                  perm, 
                                             int                  flags)
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      d_.permissions_allow (adata->try_translate (s_, tgt), id, perm, flags);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_permissions_deny  (saga::impl::void_t & ret, 
                                             saga::url            tgt, 
                                             std::string          id, 
                                             int                  perm, 
                                             int                  flags)
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      d_.permissions_deny (adata->try_translate (s_, tgt), id, perm, flags);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_permissions_check (bool      & ret, 
                                             std::string id, 
                                             int         perm)
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      ret = d_.permissions_check (id, perm);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_owner (std::string & out)
  {
    SSH_ADAPTOR_TRY
      out = d_.get_owner ();
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_group (std::string & out)
  {
    out = d_.get_group ();
  }

} // namespace

