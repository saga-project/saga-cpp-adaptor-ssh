//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/exception.hpp>

#include "sshfs_file_adaptor_dir.hpp"
#include "sshfs_file_adaptor_file.hpp"
#include "sshfs_file_adaptor.hpp"

namespace sshfs_filesystem_adaptor
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  file_cpi_impl::file_cpi_impl (proxy                * p, 
                                cpi_info       const & info,
                                saga::ini::ini const & glob_ini,
                                saga::ini::ini const & adap_ini,
                                boost::shared_ptr <saga::adaptor> adaptor)
      : file_cpi (p, info, adaptor, cpi::Noflags)
  {
    adaptor_instance_data_t adata (this);
    file_instance_data_t    idata (this);

    s_ = p->get_session ();

    init ();
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  file_cpi_impl::~file_cpi_impl (void)
  {
    // nothing to do - sshfs_ is freed automatically
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  // init() is called whenever the cpi's instance data have changed.  In that
  // case, it re-initializes the proxy directory instance which points into the
  // locally mounted sshfs name spaces, or into the local file system in
  // general. 
  //
  // init() will generally throw if the idata->location points elsewhere.
  // init() is however assumed to be atomic, so it either succeeds, or it fails
  // but then leaves the object state unchanged.  That way, the individual
  // methods don't need to attempt to recover from a failed init.  The only side
  // effect can be an additionally mounted sshfs, which is getting cleaned up by
  // the adaptor d'tor (that cleanup is optional though).
  //
  // The single argument, the url 'save', is used to restore state on failure
  //
  void file_cpi_impl::init (void)
  {
    adaptor_instance_data_t adata (this);
    file_instance_data_t    idata (this);

    if ( idata->location_.get_scheme () != "ssh" &&
         idata->location_.get_scheme () != "any" )
    {
      SAGA_LOG_DEBUG ("sshfs can not handle url:");
      SAGA_LOG_DEBUG (idata->location_.get_string ().c_str ());

      std::stringstream ss;
      ss << "Cannot handle URL scheme " << idata->location_.get_scheme ()
         << " - can only handle schemas 'ssh' or 'any'." << std::endl;
      SAGA_ADAPTOR_THROW_NO_CONTEXT (ss.str ().c_str (), 
                                     saga::adaptors::AdaptorDeclined);
    }


    // create a new api object from the changed instance data.
    bool            success = false;
    saga::exception ex ("", saga::NoSuccess);
    saga::url       new_url;

    try
    {
      new_url = adata->translate (s_, idata->location_);
      success = true;
    }
    catch ( const saga::exception & e )
    {
      ex = e;
    }

    if ( success )
    {
      try
      {
        f_ = saga::filesystem::file (adata->strip_session (s_),
                                     new_url,
                                     idata->mode_);
      }
      catch ( const saga::exception & e )
      {
        std::stringstream ss;
        ss << "Cannot handle URL 5: \n\t" << e.what () << "\n";
        SSH_ADAPTOR_RETHROW (e, ss.str ().c_str (), saga::BadParameter);
      }
    }
    else
    {
      try
      {
        new_url = adata->translate (s_, idata->location_);
        f_ = saga::filesystem::file (adata->strip_session (s_),
                                     new_url,
                                     idata->mode_);
      }
      catch ( const saga::exception & e )
      {
        // FIXME: shouldn't we merge e and ex?
        std::stringstream ss;
        ss << "Cannot handle URL 6: \n\t" << ex.what () << "\n";
        SSH_ADAPTOR_RETHROW (ex, ss.str ().c_str (), saga::BadParameter);
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_get_size (saga::off_t & size_out)
  {
    file_instance_data_t    idata (this);
    adaptor_instance_data_t adata (this);

    /// suite_instance_data_t   sdata (this);

    SSH_ADAPTOR_TRY
      size_out = f_.get_size ();
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_read (saga::ssize_t        & len_out,
                                 saga::mutable_buffer   data,
                                 saga::ssize_t          len_in)
  {
    SSH_ADAPTOR_TRY
      len_out = f_.read (data, len_in);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_write (saga::ssize_t      & len_out, 
                                  saga::const_buffer   data,
                                  saga::ssize_t        len_in)
  {
    SSH_ADAPTOR_TRY
      len_out = f_.write (data, len_in);
    SSH_ADAPTOR_CATCH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_seek (saga::off_t                 & out, 
                                 saga::off_t                   offset, 
                                 saga::filesystem::seek_mode   whence)
  {
    SSH_ADAPTOR_TRY
      out = f_.seek (offset, whence);
    SSH_ADAPTOR_CATCH;
  }

} // namespace

