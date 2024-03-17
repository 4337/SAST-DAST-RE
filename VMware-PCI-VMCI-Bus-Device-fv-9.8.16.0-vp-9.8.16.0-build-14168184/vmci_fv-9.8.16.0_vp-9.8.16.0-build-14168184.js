//!load jsprovider.dll
//.scriptrun "Z:\work\Bugs\11.02.2024 - VMWare Workstation\Files\VMware PCI VMCI Bus Device fv-9.8.16.0 vp-9.8.16.0 build-14168184\vmci - TEN !\device\Win8\DYN\vmci_ fv-9.8.16.0 vp-9.8.16.0 build-14168184.js"
//offsets are valid for vmci.sys fv-9.8.16.0 vp-9.8.16.0 build-14168184



function initializeScript()
{
}

//https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/native-objects-in-javascript-extensions-debugger-objects - jakas tam dokuemntacja api, lepsza niz nic


function bp_vp()  {
	
	 host.namespace.Debugger.Utility.Control.ExecuteCommand("r r14b = 1");
	 host.diagnostics.debugLog("vmci.sys IRP_MJ_CREATE_HANDLER pathed: \r\n_Irp->CurrentStackLocation->FileObject->FsContext->off_0x10 is set to 1\r\n");
	 
	 return false;
}

function bp_vmci_create() {
	let regs = host.currentThread.Registers.User;
	host.diagnostics.debugLog("allocated VMCIContext* address is: "+regs.rax.toString()+"\r\n");
	return false;
}

function bp_op_8_alloc_size() {
	     let regs = host.currentThread.Registers.User;
		 host.diagnostics.debugLog("vmci.sys ioctl: 0x8103204C allocation size is: "+regs.rax.toString()+"\r\n");
		 return false;
}

function  bp_op_8_alloc_ptr() {
	      let regs = host.currentThread.Registers.User; 
		  host.diagnostics.debugLog("vmci.sys ioctl: 0x8103204C user size controled allocation address is: "+regs.rax.toString()+"\r\n");
		  return false;
}

function bp_op_2_free_VMCIContext() {
	      let regs = host.currentThread.Registers.User; 
		  host.diagnostics.debugLog("vmci.sys ioctl: 0x8103204C freeing VMCIContext* address is : "+regs.rdi.toString()+"\r\n");
		  return false;
}

function  bp_op_8_copy_mem() {
	      let regs = host.currentThread.Registers.User; 
		  host.diagnostics.debugLog("vmci.sys ioctl: 0x8103204C copy memory of size: "+regs.r8.toString()+" to addr : "+regs.rcx.toString()+"\r\n");
		  return false;
}

function invokeScript()
{
	 host.namespace.Debugger.Utility.Control.ExecuteCommand("bc *");
	
	 var ba = "0x"+host.namespace.Debugger.Utility.Control.ExecuteCommand("!lmi vmci")[2].toString().trim().substring("Base Address: ".length);
	 host.diagnostics.debugLog("vmci.sys base address is: "+ba+"\r\n");
	 
	 var rde = host.parseInt64(ba).add(0x14a4);  //offset 2 RealDriverEntry
	 host.diagnostics.debugLog("vmci.sys RealDriverEntry address is: 0x"+rde.toString(16)+"\r\n");
	 
	 var imc = rde.add(0x61); //offset 2 IRP_MJ_CREATE_HANDLER 
	 
	 var disasm = host.namespace.Debugger.Utility.Code.CreateDisassembler();

     var imc_op = disasm.DisassembleInstructions(imc).First()
	 var imc_hnd = host.parseInt64(disasm.DisassembleInstructions(imc).First().Operands[1].ImmediateValue.toString()); //imc_op.Operands[1].ImmediateValue.toString());

	 host.diagnostics.debugLog("vmci.sys IRP_MJ_CREATE handler address is: "+imc_hnd.toString()+"\r\n");
	 
	 var bp_addr = imc_hnd.add(0x138); //valid for fv-9.8.18.0 vp-9.8.18.0 build-18956547, bp for modify _Irp->CurrentStackLocation->FileObject->FsContext->off_0x10 value to 1
     //host.diagnostics.debugLog(disasm.DisassembleInstructions(bp_addr).First()+"\r\n");
	 
	 host.namespace.Debugger.Utility.Control.ExecuteCommand('bp /w "@$scriptContents.bp_vp()" '+bp_addr.toString());
	
	 //rde + 0x53, offset 2 IRP_MJ_DEVICE_IO_CONTROL
	 var mdc = host.parseInt64(disasm.DisassembleInstructions(rde.add(0x53)).First().Operands[1].ImmediateValue.toString());
	 host.diagnostics.debugLog("vmci.sys IRP_MJ_DEVICE_IO_CONTROL handler address is: "+mdc.toString()+"\r\n");
	
	 //mdc + 0x68d = call Create_VMCIContext_and_put_in_global_list_sub_1400052E0
	 var create_vmci_addr = host.parseInt64(disasm.DisassembleInstructions(mdc.add(0x68d)).First().Operands[0].ImmediateValue.toString());
	 host.diagnostics.debugLog("vmci.sys Create_VMCIContext_and_put_in_global_list_sub_1400052E0 address is: "+create_vmci_addr.toString()+"\r\n");
	 
	 let VMCIContext = create_vmci_addr.add(0x65); 
	 host.namespace.Debugger.Utility.Control.ExecuteCommand('bp /w "@$scriptContents.bp_vmci_create()" '+VMCIContext.toString()); 
	
	 //szukamy w 0x8103204C alokacji naszej pamieci  mdc + 0x749
	 var io_0x8103204C = host.parseInt64(disasm.DisassembleInstructions(mdc.add(0xa7c)).First().Operands[0].ImmediateValue.toString());
	 host.diagnostics.debugLog("vmci.sys ioctl 0x8103204C handler is: "+io_0x8103204C.toString()+"\r\n");
	 
	 //io_0x8103204C = 0x17e == call IOCTLIRP_SystemBuffer_8_eq_8_sub_1400044C0
	 var io_0x8103204C_op_8 = host.parseInt64(disasm.DisassembleInstructions(io_0x8103204C.add(0x17e)).First().Operands[0].ImmediateValue.toString());
	 host.diagnostics.debugLog("vmci.sys ioctl 0x8103204C->IOCTLIRP_SystemBuffer_8_eq_8_sub_1400044C0 handler is: "+io_0x8103204C_op_8.toString()+"\r\n");
	 
	 //set bp IOCTLIRP_SystemBuffer_8_eq_8_sub_1400044C0 + 0x37 (r3 size for ExAllocatePoolWithTag) 
	 host.namespace.Debugger.Utility.Control.ExecuteCommand('bp /w "@$scriptContents.bp_op_8_alloc_size()" '+io_0x8103204C_op_8.add(0x37).toString());
	 
	 //set bp IOCTLIRP_SystemBuffer_8_eq_8_sub_1400044C0 + 0x4b (rax = pointer returned by ExAllocatePoolWithTag);
	 //bp_op_8_alloc_ptr()
	 host.namespace.Debugger.Utility.Control.ExecuteCommand('bp /w "@$scriptContents.bp_op_8_alloc_ptr()" '+io_0x8103204C_op_8.add(0x4b).toString());
	 
	 //set bp on FreeVMCIContext  (opcode 2)
	 //io_0x8103204C + 0x1a9  = call opcode 2 (free vmcicontext)
	 var io_0x8103204C_op_2 = host.parseInt64(disasm.DisassembleInstructions(io_0x8103204C.add(0x1a9)).First().Operands[0].ImmediateValue.toString());
	 host.diagnostics.debugLog("vmci.sys ioctl 0x8103204C->IOCTLIRP_SystemBuffer_8_eq_2_Free_VMCIContext_sub_140004770 handler is: "+io_0x8103204C_op_2.toString()+"\r\n");
	 
	 //io_0x8103204C_op_2+0x18f = edi = VMCIContext* 
	 host.namespace.Debugger.Utility.Control.ExecuteCommand('bp /w "@$scriptContents.bp_op_2_free_VMCIContext()" '+io_0x8103204C_op_2.add(0x18f).toString());
	 
	 //IOCTLIRP_SystemBuffer_8_eq_8_sub_1400044C0 +0xab, set bp IOCTLIRP_SystemBuffer_8_eq_8_sub_1400044C0 on CopyMemory_sub_14000E480
	 //bp_op_8_copy_mem()
	 host.namespace.Debugger.Utility.Control.ExecuteCommand('bp /w "@$scriptContents.bp_op_8_copy_mem()" '+io_0x8103204C_op_8.add(0xab).toString());
}

function uninitializeScript()
{
}

