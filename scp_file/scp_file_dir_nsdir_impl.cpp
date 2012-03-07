//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "scp_file_dir.hpp"

namespace scp_file_adaptor
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_change_dir (saga::impl::void_t & ret, 
                                      saga::url            name)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_list (std::vector <saga::url> & list, 
                                std::string               pattern, 
                                int                       flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_find (std::vector <saga::url> & list, 
                                std::string               entry, 
                                int                       flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_exists (bool    & exists, 
                                  saga::url url)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_dir (bool    & is_dir, 
                                  saga::url url)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_entry (bool    & is_entry, 
                                    saga::url url)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_link (bool    & is_link, 
                                   saga::url url)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_read_link (saga::url & ret, 
                                     saga::url   source)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_num_entries (std::size_t & num)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_entry (saga::url & ret, 
                                     std::size_t entry )
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_link (saga::impl::void_t & ret, 
                                saga::url            source, 
                                saga::url            url, 
                                int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                saga::url            src, 
                                saga::url            dst, 
                                int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_move (saga::impl::void_t & ret, 
                                saga::url            src, 
                                saga::url            dest, 
                                int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_remove (saga::impl::void_t & ret, 
                                  saga::url            url, 
                                  int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_copy_wildcard (saga::impl::void_t & ret, 
                                         std::string          source, 
                                         saga::url            dest, 
                                         int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_link_wildcard (saga::impl::void_t & ret, 
                                         std::string          source, 
                                         saga::url            dest, 
                                         int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_move_wildcard (saga::impl::void_t & ret, 
                                         std::string          source, 
                                         saga::url            dest, 
                                         int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_remove_wildcard (saga::impl::void_t & ret, 
                                           std::string          url, 
                                           int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_make_dir (saga::impl::void_t & ret, 
                                    saga::url            dest, 
                                    int                  flags)
  {
    dir_instance_data_t idata (this);

    std::cout << " ========= " << dest << std::endl;

    // FIXME: what do we need from the context?
    // FIXME: make sure that either src or target is local

    // we have to transform src/tgt like this:
    // scp://name@host:port/path/to/file -> -p port -l name host:/path/to/file
    std::string tgt = url_to_cl_ (dest);

    saga::adaptors::utils::process proc;

    proc.set_cmd  ("echo"); 

    std::vector <std::string> args;

    args.push_back ("/tmp/test");
    args.push_back ("|");
    args.push_back ("xargs");
    args.push_back ("mkdir");

    proc.set_args (args);

    // proc.set_args (sftp_bin_);
    // proc.set_args (sftp_opt_);
 // // proc.add_arg  ("-q");       // suppress warnings
    // proc.add_arg  (tgt);

    std::cout << " >> " << proc.dump () << std::endl;

    (void) proc.run_sync ();

    if ( ! proc.done () )
    {
      // SAGA_ADAPTOR_THROW ("Could not run a test ssh command", saga::NoSuccess);
      std::stringstream ss;
      ss << "scp (copy) failed: (" << proc.get_err_s () << ")";
      SAGA_ADAPTOR_THROW (ss.str (), saga::NoSuccess);
    }

  }
} // namespace

