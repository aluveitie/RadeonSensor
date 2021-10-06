//
//  StatusBarController.swift
//  Aureal
//
//  Copyright © 2021 Aluveitie All rights reserved.
//

import Foundation
import AppKit
import Cocoa
import SwiftUI


fileprivate class StatusbarView: NSView {
    private var normalLabel: [NSAttributedString.Key : NSObject]?
    private var compactLabel: [NSAttributedString.Key : NSObject]?
    private var normalValue: [NSAttributedString.Key : NSObject]?
    private var compactValue: [NSAttributedString.Key : NSObject]?
    
    var temps: [Int] = []
    
    
    func setup() {
        let compactLH: CGFloat = 6
        
        let p = NSMutableParagraphStyle()
        p.minimumLineHeight = compactLH
        p.maximumLineHeight = compactLH
        
        compactLabel = [
            NSAttributedString.Key.font: NSFont.init(name: "Monaco", size: 7.2)!,
            NSAttributedString.Key.foregroundColor: NSColor.labelColor,
            NSAttributedString.Key.paragraphStyle: p
        ]
        
        normalValue = [
            NSAttributedString.Key.font: NSFont.systemFont(ofSize: 14, weight: NSFont.Weight.regular),
            NSAttributedString.Key.foregroundColor: NSColor.labelColor,
        ]
        
        compactValue = [
            NSAttributedString.Key.font: NSFont.systemFont(ofSize: 9, weight: NSFont.Weight.semibold),
            NSAttributedString.Key.foregroundColor: NSColor.labelColor,
        ]
        
        normalLabel = [
            NSAttributedString.Key.font: NSFont.systemFont(ofSize: 13, weight: NSFont.Weight.regular),
            NSAttributedString.Key.foregroundColor: NSColor.labelColor,
        ]
    }
    
    func drawTitle(label: String, x: CGFloat) {
        let attributedString = NSAttributedString(string: label, attributes: normalLabel)
        attributedString.draw(at: NSPoint(x: 0, y: 2.5))
    }
    
    func drawCompactSingle(label: String, value: String, x: CGFloat) {
        let attributedString = NSAttributedString(string: label, attributes: compactLabel)
        attributedString.draw(in: NSRect(x: x, y: -4.5, width: 7, height: frame.height))
        
        let value = NSAttributedString(string: value, attributes: normalValue)
        value.draw(at: NSPoint(x: x + 10, y: 2.5))
    }
}

fileprivate class SingleGpuStatusbarView: StatusbarView {
    
    override func draw(_ dirtyRect: NSRect) {
        guard let context = NSGraphicsContext.current?.cgContext else { return }

        let temp: Int
        if (temps.count == 0) {
            temp = 0
        } else {
            temp = temps[0]
        }
        
        drawTitle(label: "GPU", x: 0)
        drawCompactSingle(label: "TEM", value: "\(temp)º", x: 35)
    }
}

fileprivate class MultiGpuStatusbarView: StatusbarView {
    
    var nrOfGpus: Int = 0;
    
    override func draw(_ dirtyRect: NSRect) {
        guard let context = NSGraphicsContext.current?.cgContext else { return }
               
        drawTitle(label: "GPU", x: 0)
        for i in 1...nrOfGpus {
            let temp: Int
            if (i <= temps.count) {
                temp = temps[i-1]
            } else {
                temp = 20
            }
            drawCompactSingle(label: String(format:"GP%d", i), value: "\(temp)º", x: CGFloat(35 + (i-1)*40))
        }
    }
}

fileprivate class NoGpuStatusbarView: StatusbarView {
    
    override func draw(_ dirtyRect: NSRect) {
        guard let context = NSGraphicsContext.current?.cgContext else { return }
        
        drawTitle(label: "GPU NOT FOUND", x: 0)
    }
}

class StatusBarController {
    private var statusItem: NSStatusItem!
    fileprivate var view: StatusbarView!
    
    private var popover: NSPopover
    
    private var updateTimer: Timer?
    
    private var nrOfGpus: Int
    
    init() {
        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        statusItem.isVisible = true

        nrOfGpus = RadeonModel.shared.getNrOfGpus()
        if (nrOfGpus < 1) {
            view = NoGpuStatusbarView()
            statusItem.length = 110
        } else if (nrOfGpus == 1) {
            view = SingleGpuStatusbarView()
            statusItem.length = 70
        } else {
            let multiview = MultiGpuStatusbarView()
            multiview.nrOfGpus = nrOfGpus
            view = multiview
            statusItem.length = CGFloat((35 + (nrOfGpus * 40) - 5))
        }
        view.setup()
                
        popover = NSPopover.init()
        let popupView = PopupView()
        popover.contentSize = NSSize(width: 120, height: 32)
        popover.contentViewController = NSHostingController(rootView: popupView)
        
        if let statusBarButton = statusItem.button {
            view.frame = statusBarButton.bounds
            statusBarButton.wantsLayer = true
            statusBarButton.addSubview(view)
            statusBarButton.action = #selector(togglePopover(sender:))
            statusBarButton.target = self
        }
        
        if (nrOfGpus > 0) {
            updateTimer = Timer.scheduledTimer(withTimeInterval: 2, repeats: true, block: { _ in
                self.update()
            })
        }
    }
    
    func update() {
        let temps = RadeonModel.shared.getTemps(nrOfGpus: nrOfGpus)
        
        view.temps = temps

        view.setNeedsDisplay(view.frame)
    }
    
    func dismiss() {
        updateTimer?.invalidate()
        NSStatusBar.system.removeStatusItem(statusItem!)
        statusItem = nil
    }
    
    @objc func togglePopover(sender: AnyObject) {
        if (popover.isShown) {
            popover.performClose(sender)
        } else {
            if let statusBarButton = statusItem.button {
                popover.show(relativeTo: statusBarButton.bounds, of: statusBarButton, preferredEdge: NSRectEdge.maxY)
            }
        }
    }
}

struct PopupView: View {
    var body: some View {
        Button(action: { exit(0) }) {
            Text("Exit").frame(maxWidth: .infinity, maxHeight: .infinity)
        }
            
    }
}
