//USBIPEnum_x64.Sys v1.0.0.0 (FDO)<br/>
//Sterownik FDO nie udostępnia dowiązania symbolicznego, to nie znaczy że nie można uzyskać dostępu do jego interfejsu.<br/>
<br/>
<ul>
<li><b>ioctl: 0x2A4000</b></li>
<u>Device:  BUS_EXTENDER (0x2a)</u><br/>
<u>Function: 0x0</u><br/>
<u>Access: FILE_READ_ACCESS</u><br/>
<u>Method: METHOD_BUFFERED</u><br/>
<u>Buffers size: IN == 0x10, OUT == unused</u><br/>
<u>Additional requirements: (_DEVICE_OBJECT->DeviceExtension->off_0x0C != 6)</u><br/>
<br/>

(Buffor wejściowy to elemnty nazwy urządzenia USB (https://learn.microsoft.com/en-us/windows-hardware/drivers/install/standard-usb-identifiers) klasa/podklasa/etc. 
USB\Class_%02x&SubClass_%02x&Prot_%02xZUSB\Class_%0)

informuje PNP o podłączeniu nowego urządzenia USB.
<br/>
<br/>
<li><b>ioctl: 0x2A4004</b></li>
<u>Device:  BUS_EXTENDER (0x2a)</u><br/>
<u>Function: 0x1</u><br/>
<u>Access: FILE_READ_ACCESS</u><br/>
<u>Method: METHOD_BUFFERED</u><br/>
<u>Buffers size: IN == 4, OUT == unused</u><br/>
<u>Additional requirements: (_DEVICE_OBJECT->DeviceExtension->off_0x0C != 6)</u><br/>
<br/>
IO->SystemBuffor[0] = val <= 0x7A
<br/>
informuje PNP o odłączeniu urządzenia USB.
<br/>
<br/>
<li><b>ioctl: 0x2A4008</b></li>
<u>Device:  BUS_EXTENDER (0x2a)</u><br/>
<u>Function: 0x2</u><br/>
<u>Access: FILE_READ_ACCESS</u><br/>
<u>Method: METHOD_BUFFERED</u><br/>
<u>Buffers size: IN == 0x10, OUT == unused</u><br/>
<u>Additional requirements: (_DEVICE_OBJECT->DeviceExtension->off_0x0C != 6)</u><br/>
<br/>
informuje PNP o bezpiecznym odłączeniu urządzenia USB.
<br/>
<br/>
<li><b>ioctl: 0x2A400C</b></li>
<u>Device:  BUS_EXTENDER (0x2a)</u><br/>
<u>Function: 0x3</u><br/>
<u>Access: FILE_READ_ACCESS</u><br/>
<u>Method: METHOD_BUFFERED</u><br/>
<u>Buffers size: IN == unused, OUT == 0x80</u><br/>
<u>Additional requirements: (_DEVICE_OBJECT->DeviceExtension->off_0x0C != 6)</u><br/>
<br/>
Kopiuje 0x80 bajtów z pod _DEVICE_OBJECT->DeviceExtension->off_0x30 do buffora wyjściowego.
<br/>
<br/>
<li><b>ioctl: 0x220003 (internal)</b></li>
<u>Device:  UNKNOWN (0x22)</u><br/>
<u>Function: 0x0</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>
<u>Method: METHOD_NEITHER</u><br/>
<u>Buffers size: IN == N/A , OUT == N/A</u><br/>
<u>Additional requirements: (DeviceObject->DeviceExtension->off_0x50 != 0) && (DeviceObject->DeviceExtension->off_0x8 == 0)</u><br/>
<br/>
N/A
<br/>
<br/>
<li><b>ioctl: 0x220007 (internal)</b></li>
<u>Device:  UNKNOWN (0x22)</u><br/>
<u>Function: 0x1</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>
<u>Method: METHOD_NEITHER</u><br/>
<u>Buffers size: IN == N/A , OUT == N/A</u><br/>
<u>Additional requirements: N/A</u><br/>
<br/>
N/A
<br/>
<br/>
<li><b>ioctl: 0x22000F (internal)</b></li>
<u>Device:  UNKNOWN (0x22)</u><br/>
<u>Function: 0x3</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>
<u>Method: METHOD_NEITHER</u><br/>
<u>Buffers size: IN == N/A , OUT == N/A</u><br/>
<u>Additional requirements: N/A</u><br/>
<br/>
N/A
<br/>
<br/>
<li><b>ioctl: 0x22043B (internal)</b></li>
<u>Device:  UNKNOWN (0x22)</u><br/>
<u>Function: 0x10e</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>
<u>Method: METHOD_NEITHER</u><br/>
<u>Buffers size: IN == N/A , OUT == N/A</u><br/>
<u>Additional requirements: N/A</u><br/>
<br/>
N/A
<br/>
<br/>
<li><b>IRP_MJ_WRITE</b></li>
<u>Device: N/A</u><br/>
<u>Function: N/A</u><br/>
<u>Access: N/A</u><br/>
<u>Method: N/A</u><br/>
<u>Rozmiar bufforów: OUT >= 0x30, IN = N/A</u><br/>
<u>Additional requirments (DeviceObject->DeviceExtension->off_0x8 != 0) && (DeviceObject->DeviceExtension->off_0xc != 6) && (FileObject->FsContext != nullptr) &&
                       (FileObject->FsContext->off_0x50 != 0)</u><br/>
<br/>
//(IOCTL_2A4000_Runtine) przypisuje do FileObject->FsContext (SECURE)DeviceObject->DeviceExtensions<br/>

Mn. obsługuje URB.
<br/>
<br/>
<li><b>IRP_MJ_READ</b></li>
<u>Device: N/A</u><br/>
<u>Function: N/A</u><br/>
<u>Access: N/A</u><br/>
<u>Method: N/A</u><br/>
<u>Rozmiar bufforów: OUT == N/A, IN = N/A</u><br/>
<u>Additional requirments (DeviceObject->DeviceExtension->off_0x8 != 0) && (DeviceObject->DeviceExtension->off_0xc != 6) && (FileObject->FsContext != nullptr) &&
                       (FileObject->FsContext->off_0x50 != 0)</u><br/>
<br/>
Pbsluguje prywtane kody IOCTL i URB USB.
<br/>
<br/>
<h3>WMI</h3>

SetWmiDataItem_proc	PAGE	000000000001FA84	00000068	00000038	00000038	R	.	.	.	.	.	.	.	.<br/>
SetWmiDataBlock_proc	PAGE	000000000001FAF4	0000005A	00000038	00000030	R	.	.	.	.	.	.	.	.<br/>
QueryWmiDataBlock_proc	PAGE	000000000001FB54	0000005C	00000038	00000040	R	.	.	.	.	.	.	.	.<br/>
QueryWmiRegInfo_proc	PAGE	000000000001FBB8	0000003D	00000028	00000030	R	.	.	.	.	.	.	.	.<br/>

</ul>

<h3>LINKI:</h3>

<ul>
<li><b>
<a href="https://github.com/4337/SAST-DAST-RE/blob/main/Polycom-BToE-Enumerator-1.0.0.0-(USBIPEnum_x64.Sys)/USBIPEnum_x64_interface.cpp" target="_blank">(Polycom) USBIPEnum Interface - Teraz możecie sobie atakować URB, czy co tam chcecie :)</a>
</b></li>
<li><b>
<a href="https://github.com/4337/SAST-DAST-RE/blob/main/Polycom-BToE-Enumerator-1.0.0.0-(USBIPEnum_x64.Sys)/USBIPEnum_x64.Sys.i64" target="_blank">IDA db</a>
</b></li>
</ul>