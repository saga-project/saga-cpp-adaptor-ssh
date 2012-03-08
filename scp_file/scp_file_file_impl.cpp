//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "scp_file_file.hpp"

namespace scp_file_adaptor
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
    adaptor_data_t       adata (this);
    file_instance_data_t idata (this);

    // s_ = p->get_session ();

    init_ ();

    ini_ = adap_ini.get_section ("preferences").get_entries ();
    check_ini_ ();

  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  file_cpi_impl::~file_cpi_impl (void)
  {
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_get_size (saga::off_t & size_out)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_read (saga::ssize_t        & len_out,
                                 saga::mutable_buffer   data,
                                 saga::ssize_t          len_in)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_write (saga::ssize_t      & len_out, 
                                  saga::const_buffer   data,
                                  saga::ssize_t        len_in)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_seek (saga::off_t                 & out, 
                                 saga::off_t                   offset, 
                                 saga::filesystem::seek_mode   whence)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::check_ini_ (void)
  {
    // check if ini has required entries
    if ( ini_.find ("scp_bin")  == ini_.end () ||
         ini_["scp_bin"]        == ""          || 
         ini_.find ("ssh_bin")  == ini_.end () ||
         ini_["ssh_bin"]        == ""          ||  
         ini_.find ("sftp_bin") == ini_.end () ||
         ini_["sftp_bin"]       == ""          )  
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("need path to scp, ssh and sftp in the SAGA ini",
                                     saga::NoSuccess);
    }


    // set default opts (none)
    if ( ini_.find ("scp_opt")  == ini_.end () ) { ini_["scp_opt"]  = ""; }
    if ( ini_.find ("ssh_opt")  == ini_.end () ) { ini_["ssh_opt"]  = ""; }
    if ( ini_.find ("sftp_opt") == ini_.end () ) { ini_["sftp_opt"] = ""; }

    scp_bin_   = ini_["scp_bin"];
    ssh_bin_   = ini_["ssh_bin"];
    sftp_bin_  = ini_["sftp_bin"];

    scp_opt_   = saga::adaptors::utils::split (ini_["scp_opt"],  ' ');
    ssh_opt_   = saga::adaptors::utils::split (ini_["ssh_opt"],  ' ');
    sftp_opt_  = saga::adaptors::utils::split (ini_["sftp_opt"], ' ');
  }


  void file_cpi_impl::init_ (void)
  {
    adaptor_data_t       adata (this);
    file_instance_data_t idata (this);

    if ( idata->location_.get_scheme () != "ssh"   &&
         idata->location_.get_scheme () != "scp"   &&
         idata->location_.get_scheme () != "sftp"  &&
         idata->location_.get_scheme () != "local" &&
         idata->location_.get_scheme () != "file"  &&
         idata->location_.get_scheme () != "any"  )
    {
      SAGA_LOG_DEBUG ("scp can not handle url:");
      SAGA_LOG_DEBUG (idata->location_.get_string ().c_str ());

      std::stringstream ss;
      ss << "Cannot handle URL scheme " << idata->location_.get_scheme ()
        << " - can only handle ssh/scp/sftp/local/file/any'." << std::endl;
      SAGA_ADAPTOR_THROW_NO_CONTEXT (ss.str ().c_str (), 
                                     saga::adaptors::AdaptorDeclined);
    }
  }

  std::string file_cpi_impl::url_to_cl_ (saga::url & u)
  {
    std::stringstream out;

    if ( u.get_port () > 0 )
    {
      out << "-p " << u.get_port () << " ";
    }

    if ( ! u.get_username ().empty () )
    {
      out << u.get_username () << "@";
    }

    if ( ! u.get_host ().empty () )
    {
      out << u.get_host () << ":";
    }

    out << u.get_path ();

    return out.str ();
  }

} // namespace

