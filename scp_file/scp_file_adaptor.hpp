//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_SCP_FILE_ADAPTOR_HPP
#define ADAPTORS_SCP_FILE_ADAPTOR_HPP

#include <saga/saga/url.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

namespace scp_file_adaptor 
{
  struct file_adaptor : public saga::adaptor
  {
    typedef saga::impl::v1_0::op_info         op_info;  
    typedef saga::impl::v1_0::cpi_info        cpi_info;
    typedef saga::impl::v1_0::preference_type preference_type;

    file_adaptor (void) 
    {
    }

    ~file_adaptor (void)
    {
    }

    std::string get_name (void) const
    {
      return BOOST_PP_STRINGIZE (SAGA_ADAPTOR_NAME);
    }

    /**
     * This functions registers the adaptor with the factory
     *
     * @param factory the factory where the adaptor registers
     *        its maker function and description table
     */
    saga::impl::adaptor_selector::adaptor_info_list_type 
      adaptor_register (saga::impl::session * s);
  };
} // namespace

#endif // ADAPTORS_SCP_FILE_ADAPTOR_HPP

