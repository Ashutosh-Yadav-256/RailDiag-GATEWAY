PROGRAM VehicleDiagnostic
VAR
    TemperatureC         : REAL := 20.0;
    VoltageV             : REAL := 24.0;
    DeviceWarning        : BOOL := FALSE;
    DeviceFault          : BOOL := FALSE;

    OverTemperature      : BOOL := FALSE;
    TemperatureWarning   : BOOL := FALSE;
    CriticalUndervoltage : BOOL := FALSE;
    VoltageWarning       : BOOL := FALSE;

    DiagnosticFault      : BOOL := FALSE;
    DiagnosticWarning    : BOOL := FALSE;
END_VAR

OverTemperature := TemperatureC >= 90.0;
CriticalUndervoltage := VoltageV < 18.0;

TemperatureWarning := (TemperatureC >= 75.0) AND (TemperatureC < 90.0);
VoltageWarning := (VoltageV < 21.0) AND (VoltageV >= 18.0);

DiagnosticFault := DeviceFault OR OverTemperature OR CriticalUndervoltage;
DiagnosticWarning := (NOT DiagnosticFault) AND (DeviceWarning OR TemperatureWarning OR VoltageWarning);
END_PROGRAM
