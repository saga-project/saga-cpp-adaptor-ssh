//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "scp_file_file.hpp"

namespace scp_file_adaptor
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_get_url (saga::url & url)
  {
    init_ ();

    file_instance_data_t idata (this);

    url = idata->location_;
  }

  void file_cpi_impl::sync_get_cwd  (saga::url & cwd)
  {
    init_ ();

    file_instance_data_t idata (this);

    cwd = idata->location_;

    // remove name element from path
    cwd.set_path (cwd.get_path ());
  }

  void file_cpi_impl::sync_get_name (saga::url & name)
  {
    init_ ();

    file_instance_data_t idata (this);

    // name = idata->location_.get_name ();
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void file_cpi_impl::sync_is_dir (bool & is_dir)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void file_cpi_impl::sync_is_entry  (bool & is_file)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void file_cpi_impl::sync_is_link   (bool & is_link)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void file_cpi_impl::sync_read_link (saga::url & target)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void file_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                 saga::url dest, int flags)
  {
    file_instance_data_t idata (this);

    // FIXME: what do we need from the context?
    // FIXME: make sure that either src or target is local

    // we have to transform src/tgt like this:
    // scp://name@host:port/path/to/file -> -p port -l name host:/path/to/file
    std::string src = url_to_cl_ (idata->location_);
    std::string tgt = url_to_cl_ (dest);


    saga::adaptors::utils::process proc;

    proc.set_cmd  (scp_bin_);
    proc.set_args (scp_opt_);
 // proc.add_arg  ("-q");       // suppress warnings
    proc.add_arg  (src);
    proc.add_arg  (tgt);

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

  void file_cpi_impl::sync_link (saga::impl::void_t & ret,    
                                 saga::url            dest, 
                                 int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void file_cpi_impl::sync_move (saga::impl::void_t & ret,   
                                 saga::url            dest, 
                                 int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void file_cpi_impl::sync_remove (saga::impl::void_t & ret,
                                   int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  void file_cpi_impl::sync_close (saga::impl::void_t & ret, 
                                  double               timeout)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }
} // namespace

