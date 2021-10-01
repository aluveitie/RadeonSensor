//
//  RadeonModel.swift
//  RadeonGadget
//
//  Created by Altoo on 01.10.21.
//

import Cocoa


class RadeonModel {
    static let shared = RadeonModel()
    
    private var connect: io_connect_t = 0
    
    init() {
        if !initDriver() {
            alertAndQuit(message: "Please download RadeonSensor from the release page.")
        }
    }
    
    func getTemp() -> Int {
        let temp = kernelGetUInt64(count: 1, selector: 1)
        return temp
    }
    
    private func initDriver() -> Bool {
        let serviceObject = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("RadeonSensor"))
        if serviceObject == 0 {
            return false
        }
        
        let status = IOServiceOpen(serviceObject, mach_task_self_, 0, &connect)
        print(status)
        
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
    
    private func kernelGetUInt64(count : Int, selector : UInt32) -> Int {
        var scalerOut: UInt64 = 0
        var outputCount: UInt32 = 1

        let maxStrLength = count //MaxCpu + 3
        var outputStr: [UInt64] = [UInt64](repeating: 0, count: maxStrLength)
        var outputStrCount: Int = 8/*sizeof(uint64_t)*/ * maxStrLength
        let res = IOConnectCallMethod(connect, selector, nil, 0, nil, 0,
                                      &scalerOut, &outputCount,
                                      &outputStr, &outputStrCount)
        
        if res != KERN_SUCCESS {
            print(String(cString: mach_error_string(res)))
            return 0
        }
        
        return Int(scalerOut)
    }
}
