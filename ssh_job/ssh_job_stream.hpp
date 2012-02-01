//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

#if !defined(ADAPTORS_SSH_JOB_STREAM_HPP)
#define ADAPTORS_SSH_JOB_STREAM_HPP

// stl includes
#include <iosfwd>

// saga engine includes
#include <saga/impl/engine/cpi.hpp>


///////////////////////////////////////////////////////////////////////////////
namespace impl
{
  namespace ssh_job
  {
    template <typename Base>
    class stream
      :   public Base
    {
      private:
        typedef Base base_type;

        // a saga stream has to keep alive the proxy and the cpi instance
        TR1::shared_ptr <saga::impl::v1_0::cpi> cpi_;
        TR1::shared_ptr <saga::impl::proxy>     proxy_;


      public:
        stream (saga::impl::v1_0::job_cpi * cpi, 
                std::streambuf            * buf)
          : base_type (buf),
            cpi_      (cpi->shared_from_this ()),
            proxy_    (cpi->get_proxy ()->shared_from_this ())
          {
          }
    };

  } // namespace ssh_job
  //////////////////////////////////////////////////////////////////////

} // namespace impl
////////////////////////////////////////////////////////////////////////

#endif // !defined(ADAPTORS_SSH_JOB_STREAM_HPP)

