import Combine
import CoreBluetooth

/// Connects to the case over BLE. Protocol mirrors firmware/src/ble_service.h:
/// AnimList (read, CSV names), AnimSelect (read/write/notify, uint8 index),
/// Brightness (read/write, uint8), DisplayInfo (read, [type, w, h, bpp]).
final class BLEManager: NSObject, ObservableObject {
    static let serviceUUID = CBUUID(string: "7A0B0001-63B1-4A6F-8D3A-6E1C2A5B9D01")
    static let animListUUID = CBUUID(string: "7A0B0002-63B1-4A6F-8D3A-6E1C2A5B9D01")
    static let animSelectUUID = CBUUID(string: "7A0B0003-63B1-4A6F-8D3A-6E1C2A5B9D01")
    static let brightnessUUID = CBUUID(string: "7A0B0004-63B1-4A6F-8D3A-6E1C2A5B9D01")
    static let displayInfoUUID = CBUUID(string: "7A0B0005-63B1-4A6F-8D3A-6E1C2A5B9D01")

    @Published var status = "Starting Bluetooth…"
    @Published var connected = false
    @Published var animations: [String] = []
    @Published var selected: Int = -1
    @Published var brightness: Double = 255
    @Published var displayInfo = ""

    private var central: CBCentralManager!
    private var peripheral: CBPeripheral?
    private var animSelectChr: CBCharacteristic?
    private var brightnessChr: CBCharacteristic?

    override init() {
        super.init()
        central = CBCentralManager(delegate: self, queue: .main)
    }

    func select(_ index: Int) {
        selected = index
        guard let chr = animSelectChr else { return }
        peripheral?.writeValue(Data([UInt8(index)]), for: chr, type: .withResponse)
    }

    func applyBrightness() {
        guard let chr = brightnessChr else { return }
        peripheral?.writeValue(Data([UInt8(brightness)]), for: chr, type: .withResponse)
    }

    private func startScan() {
        status = "Scanning for LED Case…"
        central.scanForPeripherals(withServices: [Self.serviceUUID])
    }
}

extension BLEManager: CBCentralManagerDelegate {
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .poweredOn:
            startScan()
        case .unauthorized:
            status = "Bluetooth permission denied. Enable it in Settings."
        case .poweredOff:
            status = "Bluetooth is off."
        default:
            status = "Bluetooth unavailable."
        }
    }

    func centralManager(_ central: CBCentralManager,
                        didDiscover peripheral: CBPeripheral,
                        advertisementData: [String: Any], rssi RSSI: NSNumber) {
        status = "Connecting to \(peripheral.name ?? "case")…"
        central.stopScan()
        self.peripheral = peripheral
        central.connect(peripheral)
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        status = "Connected"
        connected = true
        peripheral.delegate = self
        peripheral.discoverServices([Self.serviceUUID])
    }

    func centralManager(_ central: CBCentralManager,
                        didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        connected = false
        animations = []
        selected = -1
        animSelectChr = nil
        brightnessChr = nil
        startScan()
    }

    func centralManager(_ central: CBCentralManager,
                        didFailToConnect peripheral: CBPeripheral, error: Error?) {
        startScan()
    }
}

extension BLEManager: CBPeripheralDelegate {
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        for svc in peripheral.services ?? [] where svc.uuid == Self.serviceUUID {
            peripheral.discoverCharacteristics(nil, for: svc)
        }
    }

    func peripheral(_ peripheral: CBPeripheral,
                    didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        for chr in service.characteristics ?? [] {
            switch chr.uuid {
            case Self.animListUUID, Self.displayInfoUUID:
                peripheral.readValue(for: chr)
            case Self.animSelectUUID:
                animSelectChr = chr
                peripheral.readValue(for: chr)
                peripheral.setNotifyValue(true, for: chr)
            case Self.brightnessUUID:
                brightnessChr = chr
                peripheral.readValue(for: chr)
            default:
                break
            }
        }
    }

    func peripheral(_ peripheral: CBPeripheral,
                    didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        guard let data = characteristic.value else { return }
        switch characteristic.uuid {
        case Self.animListUUID:
            animations = String(decoding: data, as: UTF8.self)
                .split(separator: ",").map(String.init)
        case Self.animSelectUUID:
            if let first = data.first { selected = Int(first) }
        case Self.brightnessUUID:
            if let first = data.first { brightness = Double(first) }
        case Self.displayInfoUUID:
            if data.count >= 4 {
                let type = data[0] == 1 ? "TFT" : "LED matrix"
                displayInfo = "\(type) \(data[1])×\(data[2]), \(data[3])-bit color"
            }
        default:
            break
        }
    }
}
