//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

#ifdef  ADAPTORS_SSH_JOB_OSTREAM_HPP
#define ADAPTORS_SSH_JOB_OSTREAM_HPP


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
  class ostream : public saga::job::ostream
  {
    private:
      typedef impl::ssh_job_stream <saga::adaptors::ostream_ptr> 
              impl_type;

    public:
      template <typename Stream>
      ostream (saga::impl::v1_0::job_cpi * cpi, 
               Stream                    & child_ostream)
        : saga::job::ostream (new impl_type (cpi, child_ostream.rdbuf ()))
      {
      }
  };

} // namespace ssh_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_SSH_JOB_OSTREAM_HPP

