import SwiftUI

struct ContentView: View {
    @EnvironmentObject var ble: BLEManager

    var body: some View {
        NavigationStack {
            List {
                Section {
                    HStack {
                        Circle()
                            .fill(ble.connected ? .green : .orange)
                            .frame(width: 10, height: 10)
                        Text(ble.status)
                        Spacer()
                    }
                    if !ble.displayInfo.isEmpty {
                        Text(ble.displayInfo)
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }
                }

                if ble.connected {
                    Section("Animation") {
                        ForEach(Array(ble.animations.enumerated()), id: \.offset) { index, name in
                            Button {
                                ble.select(index)
                            } label: {
                                HStack {
                                    Text(name)
                                    Spacer()
                                    if index == ble.selected {
                                        Image(systemName: "checkmark")
                                    }
                                }
                            }
                        }
                    }

                    Section("Brightness") {
                        Slider(value: $ble.brightness, in: 5...255) { editing in
                            if !editing { ble.applyBrightness() }
                        }
                    }
                }
            }
            .navigationTitle("LED Case")
        }
    }
}
