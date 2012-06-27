//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/exception.hpp>

#include "ssh_file_adaptor_dir.hpp"
#include "ssh_file_adaptor_file.hpp"
#include "ssh_file_adaptor.hpp"

namespace ssh_filesystem_adaptor
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  dir_cpi_impl::dir_cpi_impl (proxy                * p, 
                              cpi_info       const & info,
                              saga::ini::ini const & glob_ini,
                              saga::ini::ini const & adap_ini,
                              boost::shared_ptr<saga::adaptor> adaptor)
    : directory_cpi (p, info, adaptor, cpi::Noflags)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    // keep session, to use on all for later ops
    s_ = p->get_session ();

    // adata->dump_session ("initial  session", s_);
    // adata->dump_session ("stripped session", adata->strip_session (s_));

    // initialize cpi state - may throw
    init ();
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  dir_cpi_impl::~dir_cpi_impl (void)
  {
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
  void dir_cpi_impl::init (void)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

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
    try
    {
      d_ = saga::filesystem::directory (adata->strip_session (s_),
                                        adata->try_translate (s_, idata->location_), 
                                        idata->mode_);
    }
    catch ( const saga:: exception & e )
    {
      std::stringstream ss;
      ss << "Cannot handle URL: \n\t" << e.what () << "\n";

      SSH_ADAPTOR_RETHROW (e, ss.str ().c_str (), saga::BadParameter);
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_open (saga::filesystem::file & ret, 
                                saga::url                name, 
                                int                      openmode)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    // abs or relative name?
    saga::url u = adata->make_absolute (idata->location_, name);

    // we do *not* use the stripped session here, as the target is in all
    // likelyhood an ssh file
    SSH_ADAPTOR_TRY
      ret = saga::filesystem::file (s_, u, openmode);
    SSH_ADAPTOR_CATCH
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_open_dir (saga::filesystem::directory & ret, 
                                    saga::url                     name, 
                                    int                           openmode)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    saga::url u = adata->make_absolute (idata->location_, name);

    // we do *not* use the stripped session here, as the target is in all
    // likelyhood an ssh file
    SSH_ADAPTOR_TRY
      ret = saga::filesystem::directory (s_, u, openmode);
    SSH_ADAPTOR_CATCH
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_file (bool    & is_file, 
                                   saga::url name)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    saga::url u = adata->make_absolute (idata->location_, name);

    SSH_ADAPTOR_TRY
      is_file = d_.is_file (adata->try_translate (s_, u));
    SSH_ADAPTOR_CATCH;
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_size (saga::off_t & size_out, 
                                    saga::url      name, 
                                    int            flag)
  {
    adaptor_instance_data_t   adata (this);
    directory_instance_data_t idata (this);

    saga::url u = adata->make_absolute (idata->location_, name);

    SSH_ADAPTOR_TRY
      size_out = d_.get_size (adata->try_translate (s_, u), flag);
    SSH_ADAPTOR_CATCH;
  }

} // namespace

