// swift-tools-version: 5.8
import PackageDescription
import AppleProductTypes

let package = Package(
    name: "LEDCase",
    platforms: [
        .iOS("16.0")
    ],
    products: [
        .iOSApplication(
            name: "LEDCase",
            targets: ["AppModule"],
            bundleIdentifier: "dev.elijah.ledcase",
            displayVersion: "1.0",
            bundleVersion: "1",
            accentColor: .presetColor(.indigo),
            supportedDeviceFamilies: [
                .phone
            ],
            supportedInterfaceOrientations: [
                .portrait
            ],
            capabilities: [
                .bluetoothAlways(purposeString: "Connects to the LED phone case to choose animations and set brightness.")
            ]
        )
    ],
    targets: [
        .executableTarget(
            name: "AppModule",
            path: "."
        )
    ]
)
