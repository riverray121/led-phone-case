import SwiftUI

@main
struct LEDCaseApp: App {
    @StateObject private var ble = BLEManager()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(ble)
        }
    }
}
