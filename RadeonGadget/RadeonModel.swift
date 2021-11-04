//
//  RadeonModel.swift
//  RadeonGadget
//
//  Created by Aluveitie on 01.10.21.
//

import Cocoa

class RadeonModel {
    static let shared = RadeonModel()
    
    private var connect: io_connect_t = 0
    
    init() {
        if !initDriver() {
            alertAndQuit(message: "Please download RadeonSensor from the release page.")
        }
        
        let gadgetVersion = (0, 3)
        
        let kextVersion = getKextVersion()
        if (kextVersion.1 > gadgetVersion.1) {
            alert(message: "There are updates available for RadeonSensor. Update to the latest version for more features.")
        } else if (kextVersion.1 < gadgetVersion.1 || kextVersion.0 != gadgetVersion.0) {
            alertAndQuit(message: "There are updates available for RadeonSensor. Your version is incompatible with this version of RadeonGadget.")
        }
    }
    
    func getKextVersion() -> (Int, Int) {
        var scalerOut: UInt64 = 0
        var outputCount: UInt32 = 0

        let maxStrLength = 16
        var outputStr: [CChar] = [CChar](repeating: 0, count: maxStrLength)
        var outputStrCount: Int = maxStrLength
        let _ = IOConnectCallMethod(connect, 0, nil, 0, nil, 0,
                                      &scalerOut, &outputCount,
                                      &outputStr, &outputStrCount)
        
        let kextVersion = String(cString: Array(outputStr[0...outputStrCount-1]))
        if (kextVersion.contains(".")) {
        let versionArr = kextVersion.components(separatedBy: ".")
            return (Int(versionArr[0]) ?? 0, Int(versionArr[1]) ?? 0)
        } else {
            return (0, 0);
        }
    }
    
    func getNrOfGpus() -> Int {
        var scalerOut: UInt64 = 0
        var outputCount: UInt32 = 1

        var outputStr: [UInt64] = [UInt64]()
        var outputStrCount: Int = 0
        let _ = IOConnectCallMethod(connect, 1, nil, 0, nil, 0,
                                       &scalerOut, &outputCount,
                                       &outputStr, &outputStrCount)
            
        return Int(scalerOut)
    }
    
    func getTemps(nrOfGpus: Int) -> [Int] {
        var scalerOut: UInt64 = 0
        var outputCount: UInt32 = 0

        var outputStr: [UInt16] = [UInt16](repeating: 0, count: nrOfGpus)
        var outputStrCount: Int = 2 /* sizeof(UInt16) */ * nrOfGpus
        let _ = IOConnectCallMethod(connect, 2, nil, 0, nil, 0,
                                      &scalerOut, &outputCount,
                                      &outputStr, &outputStrCount)
        
        return outputStr.map{ Int($0) }
    }
    
    private func initDriver() -> Bool {
        let serviceObject = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("RadeonSensor"))
        if serviceObject == 0 {
            return false
        }
        
        let status = IOServiceOpen(serviceObject, mach_task_self_, 0, &connect)
        
        return status == KERN_SUCCESS
    }
    
    private func alertAndQuit(message : String){
        let alert = NSAlert()
        alert.messageText = "No RadeonSensor Found!"
        alert.informativeText = message
        alert.alertStyle = .critical
        alert.addButton(withTitle: "Quit")
        alert.addButton(withTitle: "Quit and Download")
        let res = alert.runModal()
        
        if res == .alertSecondButtonReturn {
            NSWorkspace.shared.open(URL(string: "https://github.com/aluveitie/RadeonSensor")!)
        }
        
        NSApplication.shared.terminate(self)
    }
    
    private func alert(message : String){
        let alert = NSAlert()
        alert.messageText = "Kext Update Available"
        alert.informativeText = message
        alert.alertStyle = .warning
        alert.addButton(withTitle: "Later")
        alert.addButton(withTitle: "Download")
        let res = alert.runModal()
        
        if res == .alertSecondButtonReturn {
            NSWorkspace.shared.open(URL(string: "https://github.com/aluveitie/RadeonSensor")!)
        }
    }
}
