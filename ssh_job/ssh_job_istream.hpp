//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

#if !defined(ADAPTORS_SSH_JOB_ISTREAM_HPP)
#define ADAPTORS_SSH_JOB_ISTREAM_HPP

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/job.hpp>

// adaptor includes
#include "ssh_job_stream.hpp"


////////////////////////////////////////////////////////////////////////
namespace ssh_job 
{
  class istream : public saga::job::istream
  {
    private:
      typedef impl::ssh_job::stream <saga::adaptors::istream_ptr> 
              impl_type;

    public:
      template <typename Stream>
      istream (saga::impl::v1_0::job_cpi * cpi, 
               Stream                    & child_istream)
        : saga::job::istream (new impl_type (cpi, child_istream.rdbuf ()))
      {
      }
  };

} // namespace ssh_job
////////////////////////////////////////////////////////////////////////

#endif // !defined(ADAPTORS_SSH_JOB_ISTREAM_HPP)

