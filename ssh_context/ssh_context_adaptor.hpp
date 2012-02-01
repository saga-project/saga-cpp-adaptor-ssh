//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_SSH_CONTEXT_ADAPTOR_HPP
#define ADAPTORS_SSH_CONTEXT_ADAPTOR_HPP

#include <map>
#include <vector>

#include <saga/saga/util.hpp>
#include <saga/saga/types.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/context_cpi_instance_data.hpp>

#include <saga/impl/engine/proxy.hpp>
#include <saga/impl/context_cpi.hpp>

namespace ssh_context_adaptor 
{
  ///////////////////////////////////////////////////////////////////////////
  //    
  struct context_adaptor : public saga::adaptor
  {
    typedef saga::impl::v1_0::op_info         op_info;  
    typedef saga::impl::v1_0::cpi_info        cpi_info;
    typedef saga::impl::v1_0::preference_type preference_type;

    context_adaptor (void) 
    {
    }

    ~context_adaptor (void)
    {
    }


    bool init (saga::impl::session  * session,
               saga::ini::ini const & glob_ini, 
               saga::ini::ini const & adap_ini)
    { 
      // we don't need to init anything, really
      return true; 
    }
    

    std::string get_name (void) const
    {
      return BOOST_PP_STRINGIZE (SAGA_ADAPTOR_NAME);
    }

    saga::impl::adaptor_selector::adaptor_info_list_type 
      adaptor_register (saga::impl::session * s);
  };
  //
  ///////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////////
  //
  class context_cpi_impl 
    : public saga::adaptors::v1_0::context_cpi <context_cpi_impl>
  {
    private:
      typedef saga::adaptors::v1_0::context_cpi <context_cpi_impl> base_cpi;

      class cert_info_t {
        public:
          bool         success;
          std::string  errormessage;
          std::string  private_key;
          std::string  public_key;
          std::string  userid;

          void dump (void)
          {
            std::cout << " ---------------------------------" << std::endl;
            std::cout << " success       : " << success       << std::endl;
            std::cout << " errormessage  : " << errormessage  << std::endl;
            std::cout << " private_key   : " << private_key   << std::endl;
            std::cout << " public_key    : " << public_key    << std::endl;
            std::cout << " userid        : " << userid        << std::endl;
            std::cout << " ---------------------------------" << std::endl;
          }
      };

      cert_info_t get_cert_info (std::string proxy_path = "");

    public:    
      typedef base_cpi::mutex_type mutex_type;

      // constructor of the context CPI
      context_cpi_impl (proxy                * p, 
                        cpi_info       const & info, 
                        saga::ini::ini const & glob_ini, 
                        saga::ini::ini const & adap_ini,
                        TR1::shared_ptr <saga::adaptor> adaptor);

      // destructor of the file adaptor 
      ~context_cpi_impl (void);

      // context functions
      void sync_set_defaults (saga::impl::void_t &);
  };  
  //
  ///////////////////////////////////////////////////////////////////////////

}

#endif // ADAPTORS_SSH_CONTEXT_ADAPTOR_HPP

