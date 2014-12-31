//===-- DynamicLoaderHexagon.h ----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_DynamicLoaderHexagon_H_
#define liblldb_DynamicLoaderHexagon_H_

// C Includes
// C++ Includes
// Other libraries and framework includes
#include "lldb/Breakpoint/StoppointCallbackContext.h"
#include "lldb/Target/DynamicLoader.h"

#include "HexagonDYLDRendezvous.h"

class DynamicLoaderHexagonDYLD : public lldb_private::DynamicLoader
{
public:

    static void
    Initialize();

    static void
    Terminate();

    static lldb_private::ConstString
    GetPluginNameStatic();

    static const char *
    GetPluginDescriptionStatic();

    static lldb_private::DynamicLoader *
    CreateInstance(lldb_private::Process *process, bool force);

    DynamicLoaderHexagonDYLD(lldb_private::Process *process);

    virtual
    ~DynamicLoaderHexagonDYLD();

    //------------------------------------------------------------------
    // DynamicLoader protocol
    //------------------------------------------------------------------

    virtual void
    DidAttach();

    virtual void
    DidLaunch();

    virtual lldb::ThreadPlanSP
    GetStepThroughTrampolinePlan(lldb_private::Thread &thread,
                                 bool stop_others);

    virtual lldb_private::Error
    CanLoadImage();

    virtual lldb::addr_t
    GetThreadLocalData (const lldb::ModuleSP module, const lldb::ThreadSP thread);

    //------------------------------------------------------------------
    // PluginInterface protocol
    //------------------------------------------------------------------
    virtual lldb_private::ConstString
    GetPluginName();

    virtual uint32_t
    GetPluginVersion();

    virtual void
    GetPluginCommandHelp(const char *command, lldb_private::Stream *strm);

    virtual lldb_private::Error
    ExecutePluginCommand(lldb_private::Args &command, lldb_private::Stream *strm);

    virtual lldb_private::Log *
    EnablePluginLogging(lldb_private::Stream *strm, lldb_private::Args &command);

protected:
    /// Runtime linker rendezvous structure.
    HexagonDYLDRendezvous m_rendezvous;

    /// Virtual load address of the inferior process.
    lldb::addr_t m_load_offset;

    /// Virtual entry address of the inferior process.
    lldb::addr_t m_entry_point;

    /// Rendezvous breakpoint.
    lldb::break_id_t m_dyld_bid;

    /// Loaded module list. (link map for each module)
    std::map<lldb::ModuleWP, lldb::addr_t, std::owner_less<lldb::ModuleWP>> m_loaded_modules;

    /// Enables a breakpoint on a function called by the runtime
    /// linker each time a module is loaded or unloaded.
    bool
    SetRendezvousBreakpoint();

    /// Callback routine which updates the current list of loaded modules based
    /// on the information supplied by the runtime linker.
    static bool
    RendezvousBreakpointHit(void *baton, 
                            lldb_private::StoppointCallbackContext *context, 
                            lldb::user_id_t break_id, 
                            lldb::user_id_t break_loc_id);
    
    /// Helper method for RendezvousBreakpointHit.  Updates LLDB's current set
    /// of loaded modules.
    void
    RefreshModules();

    /// Updates the load address of every allocatable section in @p module.
    ///
    /// @param module The module to traverse.
    ///
    /// @param link_map_addr The virtual address of the link map for the @p module.
    ///
    /// @param base_addr The virtual base address @p module is loaded at.
    void
    UpdateLoadedSections(lldb::ModuleSP module,
                         lldb::addr_t link_map_addr,
                         lldb::addr_t base_addr);

    /// Removes the loaded sections from the target in @p module.
    ///
    /// @param module The module to traverse.
    void
    UnloadSections(const lldb::ModuleSP module);

    /// Locates or creates a module given by @p file and updates/loads the
    /// resulting module at the virtual base address @p base_addr.
    lldb::ModuleSP
    LoadModuleAtAddress(const lldb_private::FileSpec &file, lldb::addr_t link_map_addr, lldb::addr_t base_addr);

    /// Callback routine invoked when we hit the breakpoint on process entry.
    ///
    /// This routine is responsible for resolving the load addresses of all
    /// dependent modules required by the inferior and setting up the rendezvous
    /// breakpoint.
    static bool
    EntryBreakpointHit(void *baton, 
                       lldb_private::StoppointCallbackContext *context, 
                       lldb::user_id_t break_id, 
                       lldb::user_id_t break_loc_id);

    /// Helper for the entry breakpoint callback.  Resolves the load addresses
    /// of all dependent modules.
    void
    LoadAllCurrentModules();

    /// Computes a value for m_load_offset returning the computed address on
    /// success and LLDB_INVALID_ADDRESS on failure.
    lldb::addr_t
    ComputeLoadOffset();

    /// Computes a value for m_entry_point returning the computed address on
    /// success and LLDB_INVALID_ADDRESS on failure.
    lldb::addr_t
    GetEntryPoint();

    /// Checks to see if the target module has changed, updates the target
    /// accordingly and returns the target executable module.
    lldb::ModuleSP
    GetTargetExecutable();

    /// return the address of the Rendezvous breakpoint
    lldb::addr_t
    FindRendezvousBreakpointAddress( );

private:
    DISALLOW_COPY_AND_ASSIGN(DynamicLoaderHexagonDYLD);

    const lldb_private::SectionList *
    GetSectionListFromModule(const lldb::ModuleSP module) const;
};

#endif  // liblldb_DynamicLoaderHexagonDYLD_H_
