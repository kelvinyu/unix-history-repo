//===-- HistoryUnwind.h -----------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_HistoryUnwind_h_
#define liblldb_HistoryUnwind_h_

// C Includes
// C++ Includes
#include <vector>

// Other libraries and framework includes
// Project includes
#include "lldb/lldb-private.h"
#include "lldb/Host/Mutex.h"
#include "lldb/Target/Unwind.h"

namespace lldb_private {

class HistoryUnwind : public lldb_private::Unwind
{
public:
    HistoryUnwind (Thread &thread, std::vector<lldb::addr_t> pcs, bool stop_id_is_valid);

    ~HistoryUnwind() override;

protected:
    void
    DoClear() override;

    lldb::RegisterContextSP
    DoCreateRegisterContextForFrame(StackFrame *frame) override;

    bool
    DoGetFrameInfoAtIndex(uint32_t frame_idx,
                          lldb::addr_t& cfa, 
                          lldb::addr_t& pc) override;
    uint32_t
    DoGetFrameCount() override;

private:

    std::vector<lldb::addr_t>   m_pcs;
    bool                        m_stop_id_is_valid;
};

} // namespace lldb_private

#endif // liblldb_HistoryUnwind_h_
