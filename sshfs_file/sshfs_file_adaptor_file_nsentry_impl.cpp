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
  void file_cpi_impl::sync_get_url (saga::url & url)
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      url = adata->try_translate_back (s_, f_.get_url ());
    SSH_ADAPTOR_CATCH;
  }

  void file_cpi_impl::sync_get_cwd  (saga::url & cwd)
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      cwd = adata->try_translate_back (s_, f_.get_cwd ());
    SSH_ADAPTOR_CATCH;
  }

  void file_cpi_impl::sync_get_name (saga::url & name)
  {
    SSH_ADAPTOR_TRY
      name = f_.get_name ();
    SSH_ADAPTOR_CATCH;
  }

  void file_cpi_impl::sync_is_dir (bool & is_dir)
  {
    SSH_ADAPTOR_TRY
      is_dir = f_.is_dir ();
    SSH_ADAPTOR_CATCH;
  }

  void file_cpi_impl::sync_is_entry  (bool & is_entry)
  {
    SSH_ADAPTOR_TRY
      is_entry = f_.is_entry ();
    SSH_ADAPTOR_CATCH;
  }

  void file_cpi_impl::sync_is_link   (bool & is_link)
  {
    SSH_ADAPTOR_TRY
      is_link = f_.is_link ();
    SSH_ADAPTOR_CATCH;
  }

  void file_cpi_impl::sync_read_link (saga::url & target)
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      target = adata->try_translate_back (s_, f_.read_link ());
    SSH_ADAPTOR_CATCH;
  }

  void file_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                 saga::url            dest, 
                                 int                  flags)
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      f_.copy (adata->try_translate (s_, dest), flags);
    SSH_ADAPTOR_CATCH;
  }

  void file_cpi_impl::sync_link (saga::impl::void_t & ret,    
                                 saga::url            dest, 
                                 int                  flags)
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      f_.link (adata->try_translate (s_, dest), flags);
    SSH_ADAPTOR_CATCH;
  }

  void file_cpi_impl::sync_move (saga::impl::void_t & ret,   
                                 saga::url            dest, 
                                 int                  flags)
  {
    adaptor_instance_data_t adata (this);
    file_instance_data_t    idata (this);

    SSH_ADAPTOR_TRY
      f_.move (adata->try_translate (s_, dest), flags);
    SSH_ADAPTOR_CATCH;

    // we reflect the move in our own state.  Now, this is actually not atomic
    // anymore, as the previous fs operation may have succeeded, but the init
    // may still fail.  I don't see how that can happen though, thus we ignore
    // that possible source of inconsistent state.
    idata->location_ = dest;
    init ();
  }

  // remove should also close the file, of course.  We assume this is managed by
  // f_
  void file_cpi_impl::sync_remove (saga::impl::void_t & ret,
                                   int                  flags)
  {
    SSH_ADAPTOR_TRY
      f_.remove (flags);
    SSH_ADAPTOR_CATCH;
  }


  void file_cpi_impl::sync_close (saga::impl::void_t & ret, 
                                  double               timeout)
  {
    SSH_ADAPTOR_TRY
      f_.close (timeout);
    SSH_ADAPTOR_CATCH;
  }

} // namespace

