//
//  main.swift
//  RadeonGadget
//
//  Created by Aluveitie on 01.10.21.
//

import AppKit

let app = NSApplication.shared
let delegate = AppDelegate()
app.delegate = delegate

_ = NSApplicationMain(CommandLine.argc, CommandLine.unsafeArgv)
