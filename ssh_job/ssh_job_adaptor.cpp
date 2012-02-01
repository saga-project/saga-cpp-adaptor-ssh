//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/config.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

// adaptor includes
#include "ssh_job_adaptor.hpp"
#include "ssh_job_service.hpp"
#include "ssh_job.hpp"

SAGA_ADAPTOR_REGISTER (ssh_job::adaptor);


////////////////////////////////////////////////////////////////////////
namespace ssh_job
{
  // register function for the SAGA engine
  saga::impl::adaptor_selector::adaptor_info_list_type
    adaptor::adaptor_register (saga::impl::session * s)
  {
    // list of implemented cpi's
    saga::impl::adaptor_selector::adaptor_info_list_type list;

    // create empty preference list
    preference_type prefs; 

    // create file adaptor infos (each adaptor instance gets its own uuid)
    // and add cpi_infos to list
    job_service_cpi_impl::register_cpi (list, prefs, adaptor_uuid_);
    job_cpi_impl::register_cpi         (list, prefs, adaptor_uuid_);

    // and return list
    return (list);
  }

  bool adaptor::init (saga::impl::session  * s,
                      saga::ini::ini const & glob_ini, 
                      saga::ini::ini const & adap_ini)
  { 
    return true; 
  }
  

  void adaptor::dump_context (saga::context c)
  {
    std::vector <std::string> attribs = c.list_attributes ();

    std::cout << " ------------------------------ " << std::endl;

    for ( unsigned int i = 0; i < attribs.size (); i++ )
    {
      std::cout << " " << attribs[i] << " \t: ";

      if ( c.attribute_is_vector (attribs[i]) )
      {
        std::vector <std::string> vals = c.get_vector_attribute (attribs[i]);

        for ( unsigned int j = 0; j < vals.size (); j++ )
        {
          std::cout << vals[j] << " ";
        }
      }
      else
      {
        std::string val = c.get_attribute (attribs[i]);
        std::cout << val << " ";
      }

      std::cout << std::endl;
    }

    std::cout << " ------------------------------ " << std::endl;
  }


  // translate an ssh jobid into a fork one, and vice versa
  // aka
  // [ssh://...]-[...]   -->  [fork://...]-[...]
  // [fork://...]-[...]  -->  [ssh://...]-[...]
  std::string adaptor::translate_jobid (const std::string & in, 
                                        const std::string & host)
  {
    // code comments based on
    // "[ssh://remote.host.net/]-[[fork://localhost/]-[1234]]"

    // find top level '['
    size_t one = in.find ('[', 0);
    size_t two = in.find ('[', one + 1);

    // return empty if brackets were not found
    if ( one == std::string::npos ||
         two == std::string::npos )
    {
      return "";
    }

    // extract 'ssh://remote.host.net/'
    std::string s_one = in.substr (one + 1, two - 3);

    // extract the content of the second bracket pair
    // 'fork://localhost/]-[1234]'
    std::string s_two = in.substr (two + 1, in.size () - two - 2);

    // create an url out of the first part, representing the js rm url
    saga::url rm (s_one);

    if ( rm.get_scheme () == "gsissh" ||
         rm.get_scheme () == "ssh"    )
    {
      return s_two;
    }
    else if ( rm.get_scheme () == "fork" )
    {
      // we assume that gsissh is not used locally (for whatever reason)
      std::string out ("[ssh://" + host + "]-[");

      out += in + "]";

      return (out);
    }

    // if no scheme is matching, don't translate.
    return "";
  }

} // namespace ssh_job
////////////////////////////////////////////////////////////////////////

