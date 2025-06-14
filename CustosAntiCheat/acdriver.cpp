#include <ntddk.h>
#include "../Shared/ioctl.h"

PDEVICE_OBJECT g_DeviceObject = NULL;
HANDLE g_HeartbeatThread = NULL;
LARGE_INTEGER g_LastHeartbeat;

VOID HeartbeatMonitor(PVOID Context) {
    UNREFERENCED_PARAMETER(Context);

    while (TRUE) {
        LARGE_INTEGER now;
        KeQuerySystemTime(&now);

        LONGLONG diffMs = (now.QuadPart - g_LastHeartbeat.QuadPart) / 10000;
        if (diffMs > 3000) {
            DbgPrint("[SENTINEL]: Heartbeat timeout (%lld ms)\n", diffMs);
        }

        LARGE_INTEGER interval;
        interval.QuadPart = -1 * 10000 * 1000; // 1 second
        KeDelayExecutionThread(KernelMode, FALSE, &interval);
    }
}

NTSTATUS DriverDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;
    static AC_POLICY g_Policy;

    if (stack->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
        switch (stack->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_AC_HEARTBEAT:
            KeQuerySystemTime(&g_LastHeartbeat);
            break;
        case IOCTL_AC_SET_POLICY:
            if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(AC_POLICY)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }
            RtlCopyMemory(&g_Policy, Irp->AssociatedIrp.SystemBuffer, sizeof(AC_POLICY));
            DbgPrint("AC: Policy updated (version %lu)\n", g_Policy.Version);
            break;
        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING sym = RTL_CONSTANT_STRING(SYMLINK_NAME);
    IoDeleteSymbolicLink(&sym);
    IoDeleteDevice(g_DeviceObject);
    DbgPrint("[SENTINEL]: Driver unloaded\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);
    NTSTATUS status;
    UNICODE_STRING dev = RTL_CONSTANT_STRING(DEVICE_NAME);
    UNICODE_STRING sym = RTL_CONSTANT_STRING(SYMLINK_NAME);

    status = IoCreateDevice(DriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, 0, FALSE, &g_DeviceObject);
    if (!NT_SUCCESS(status)) return status;

    IoCreateSymbolicLink(&sym, &dev);

    for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
        DriverObject->MajorFunction[i] = DriverDispatch;

    DriverObject->DriverUnload = DriverUnload;

    KeQuerySystemTime(&g_LastHeartbeat);
    PsCreateSystemThread(&g_HeartbeatThread, THREAD_ALL_ACCESS, NULL, NULL, NULL, HeartbeatMonitor, NULL);

    DbgPrint("[SENTINEL]: Driver loaded\n");
    return STATUS_SUCCESS;
}