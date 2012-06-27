//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "ssh_file_adaptor_dir.hpp"

namespace ssh_filesystem_adaptor
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_change_dir (saga::impl::void_t & ret, 
                                      saga::url            name)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    // keep original location for recover on error.
    // Note that this instance is locked via idata.
    saga::url save = idata->location_;

    // now we set our state to the absolute target location, and re-init
    idata->location_ = adata->make_absolute (idata->location_, name);

    try
    {
      init (); // may throw
    }
    catch ( const saga::exception & e )
    {
      // retore old state
      idata->location_ = save;

      // forward error
      throw e;
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_list (std::vector <saga::url> & list, 
                                std::string               pattern, 
                                int                       flags)
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      list = d_.list (pattern, flags);
    SSH_ADAPTOR_CATCH;

    // we need to translate the entries back into the ssh namespace, if possible
    for ( unsigned int i = 0; i < list.size (); i++ )
    {
      list[i] = adata->try_translate_back (s_, list[i]);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_find (std::vector <saga::url> & list, 
                                std::string               entry, 
                                int                       flags)
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      list = d_.find (entry, flags);
    SSH_ADAPTOR_CATCH;

    // we need to translate the entries back into the ssh namespace, if possible
    for ( unsigned int i = 0; i < list.size (); i++ )
    {
      list[i] = adata->try_translate_back (s_, list[i]);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_exists (bool    & exists, 
                                  saga::url name)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    saga::url u  = adata->make_absolute (idata->location_, name);
    sshfs_t   fs = adata->get_sshfs     (s_, u);

    SSH_ADAPTOR_TRY
      exists = d_.exists (fs->translate (u));
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_dir (bool    & is_dir, 
                                  saga::url name)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    saga::url u  = adata->make_absolute (idata->location_, name);
    sshfs_t   fs = adata->get_sshfs     (s_, u);

    SSH_ADAPTOR_TRY
      is_dir = d_.is_dir (fs->translate (u));
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_entry (bool    & is_entry, 
                                    saga::url name)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    saga::url u  = adata->make_absolute (idata->location_, name);
    sshfs_t   fs = adata->get_sshfs     (s_, u);

    SSH_ADAPTOR_TRY
      is_entry = d_.is_entry (fs->translate (u));
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_link (bool    & is_link, 
                                   saga::url name)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    saga::url u  = adata->make_absolute (idata->location_, name);
    sshfs_t   fs = adata->get_sshfs     (s_, u);

    SSH_ADAPTOR_TRY
      is_link = d_.is_link (fs->translate (u));
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_read_link (saga::url & ret, 
                                     saga::url   name)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    saga::url u  = adata->make_absolute (idata->location_, name);
    sshfs_t   fs = adata->get_sshfs     (s_, u);

    saga::url tgt;
    SSH_ADAPTOR_TRY
      tgt = d_.read_link (fs->translate (u));
    SSH_ADAPTOR_CATCH;

    // try to translate back into ssh namespace
    ret = adata->try_translate_back (s_, tgt);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_num_entries (std::size_t & num)
  {
    SSH_ADAPTOR_TRY
      num = d_.get_num_entries ();
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_entry (saga::url & ret, 
                                     std::size_t entry )
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      ret = adata->try_translate_back (s_, d_.get_entry (entry));
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_link (saga::impl::void_t & ret, 
                                saga::url            src, 
                                saga::url            tgt, 
                                int                  flags)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    SSH_ADAPTOR_TRY
      d_.link (adata->try_translate (s_, adata->make_absolute (idata->location_, src)), 
               adata->try_translate (s_, adata->make_absolute (idata->location_, tgt)), 
               flags);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                saga::url            src, 
                                saga::url            tgt, 
                                int                  flags)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    SSH_ADAPTOR_TRY
      d_.copy (adata->try_translate (s_, adata->make_absolute (idata->location_, src)), 
               adata->try_translate (s_, adata->make_absolute (idata->location_, tgt)), 
               flags);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_move (saga::impl::void_t & ret, 
                                saga::url            src, 
                                saga::url            tgt, 
                                int                  flags)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    SSH_ADAPTOR_TRY
      d_.move (adata->try_translate (s_, adata->make_absolute (idata->location_, src)), 
               adata->try_translate (s_, adata->make_absolute (idata->location_, tgt)), 
               flags);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_remove (saga::impl::void_t & ret, 
                                  saga::url            name, 
                                  int                  flags)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    SSH_ADAPTOR_TRY
      d_.remove (adata->try_translate (s_, adata->make_absolute (idata->location_, name)), 
                 flags);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_copy_wildcard (saga::impl::void_t & ret, 
                                         std::string          source, 
                                         saga::url            dest, 
                                         int                  flags)
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      d_.copy (source, 
               adata->try_translate (s_, dest), 
               flags);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_link_wildcard (saga::impl::void_t & ret, 
                                         std::string          source, 
                                         saga::url            dest, 
                                         int                  flags)
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      d_.link (source, 
               adata->try_translate (s_, dest), 
               flags);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_move_wildcard (saga::impl::void_t & ret, 
                                         std::string          source, 
                                         saga::url            dest, 
                                         int                  flags)
  {
    adaptor_instance_data_t adata (this);

    SSH_ADAPTOR_TRY
      d_.move (source, 
               adata->try_translate (s_, dest), 
               flags);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_remove_wildcard (saga::impl::void_t & ret, 
                                           std::string          name, 
                                           int                  flags)
  {
    adaptor_instance_data_t adata (this);

    d_.remove (adata->translate (s_, name), flags);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_make_dir (saga::impl::void_t & ret, 
                                    saga::url            name, 
                                    int                  flags)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    d_.make_dir (adata->translate (s_, adata->make_absolute (idata->location_, name)), 
                 flags);
  }

} // namespace

