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
                    if ble.sceneCount > 0 && ble.sceneCount < ble.animations.count {
                        Section("Scenes (128×128)") {
                            animationRows(0..<ble.sceneCount)
                        }
                        Section("Low-res (8×8)") {
                            animationRows(ble.sceneCount..<ble.animations.count)
                        }
                    } else {
                        Section("Animation") {
                            animationRows(0..<ble.animations.count)
                        }
                    }

                    sliderSection("Brightness", value: $ble.brightness, in: 5...255,
                                  apply: ble.applyBrightness)
                    sliderSection(speedTitle, value: $ble.speed, in: 4...128,
                                  apply: ble.applySpeed)
                }
            }
            .navigationTitle("LED Case")
        }
    }

    private var speedTitle: String {
        let mult = ble.speed / BLEManager.speedOne
        let text = mult == mult.rounded()
            ? String(format: "%.0f", mult)
            : String(format: "%.2f", mult)
        return "Speed (\(text)×)"
    }

    private func sliderSection(_ title: String, value: Binding<Double>,
                               in range: ClosedRange<Double>,
                               apply: @escaping () -> Void) -> some View {
        Section(title) {
            Slider(value: value, in: range) { editing in
                if !editing { apply() }
            }
        }
    }

    private func animationRows(_ range: Range<Int>) -> some View {
        ForEach(range, id: \.self) { index in
            Button {
                ble.select(index)
            } label: {
                HStack {
                    Text(ble.animations[index])
                    Spacer()
                    if index == ble.selected {
                        Image(systemName: "checkmark")
                    }
                }
            }
        }
    }
}
