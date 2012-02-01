//  Copyright (c) 2007-2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#ifndef SAGA_ADAPTOR_SSH_SUITE_HPP
#define SAGA_ADAPTOR_SSH_SUITE_HPP

#include <saga/saga.hpp>
#include <saga/saga/adaptors/adaptor_suite.hpp>

#include <boost/spirit/core/non_terminal/impl/static.hpp>

#include "../config/config.hpp"

namespace ssh
{
  class suite : public saga::adaptor_suite
  {
    public:
      // this c'tor is only called once, and is thread safe
      suite (void)
      {
        SAGA_VERBOSE (SAGA_VERBOSE_LEVEL_ALWAYS) 
        {
          std::cout << " === ssh::suite c'tor" << std::endl; 
        }
      }
  };
}

#endif // SAGA_ADAPTOR_SSH_SUITE_HPP

