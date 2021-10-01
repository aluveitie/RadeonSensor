//
//  AppDelegate.swift
//  RadeonGadget
//
//  Created by Altoo on 24.09.21.
//

import Cocoa

class AppDelegate: NSObject, NSApplicationDelegate {
    
    var statusBar: StatusBarController?
    
    static var shared: AppDelegate {
        NSApp.delegate as! AppDelegate
    }

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        print("launching")
        NSApp.setActivationPolicy(NSApplication.ActivationPolicy.accessory)
        statusBar = StatusBarController.init()
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        
    }

    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {
        return true
    }


}

