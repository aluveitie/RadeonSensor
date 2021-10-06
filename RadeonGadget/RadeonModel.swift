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
}
