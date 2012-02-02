//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/adaptors/adaptor.hpp>

#include "scp_file_adaptor.hpp"
#include "scp_file_dir.hpp"
#include "scp_file_file.hpp"

namespace scp_file_adaptor
{
  SAGA_ADAPTOR_REGISTER (file_adaptor);

  ///////////////////////////////////////////////////////////////////////////////
  //
  saga::impl::adaptor_selector::adaptor_info_list_type
    file_adaptor::adaptor_register (saga::impl::session * s)
  {
    // list of implemented cpi's
    saga::impl::adaptor_selector::adaptor_info_list_type infos;
    preference_type prefs; 

    // tell the engine to only call this ssh adaptor if an ssh context is
    // available
    prefs.set (saga::attributes::context_type, "ssh");

    file_cpi_impl::register_cpi (infos, prefs, adaptor_uuid_);
    dir_cpi_impl::register_cpi  (infos, prefs, adaptor_uuid_);

    return infos;
  }

  bool file_adaptor::is_absolute (const saga::url & u)
  {
    if ( u.get_scheme   ().empty ()  &&
         u.get_host     ().empty ()  &&
         u.get_username ().empty ()  &&
         u.get_password ().empty ()  &&
         u.get_port     ()    == -1  &&
         u.get_path     ()[0] != '/' )
    {
      return false;
    }
    
    return true;
  }


  saga::url file_adaptor::make_absolute (const saga::url & base, 
                                         const saga::url & u)
  {
    if ( is_absolute (u) )
    {
      return u;
    }

    // relative 
    saga::url ret (base); // copy scheme, host, etc
    ret.set_path  (base.get_path () + "/" + u.get_path ());

    return ret;
  }

} // namespace

