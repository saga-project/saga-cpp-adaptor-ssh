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

    file_cpi_impl::register_cpi        (infos, prefs, adaptor_uuid_);
    dir_cpi_impl::register_cpi         (infos, prefs, adaptor_uuid_);

    return infos;
  }
} // namespace

