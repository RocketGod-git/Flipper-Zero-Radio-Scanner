# Flipper Zero Radio Scanner

A radio scanner application for the Flipper Zero, designed to scan SubGHz radio frequencies using the built-in CC1101 and plays them over the device's speaker.

## 📻 Features

- **Frequency Scanning:** Automatically scans through frequencies in valid SubGHz ranges.
- **Real-Time RSSI Display:** Shows the received signal strength indicator (RSSI) updated periodically.
- **Sensitivity Configuration:** Allows you to adjust the sensitivity threshold for signal detection.
- **Sound Modes:**
  - **OFF:** No audio feedback.
  - **ON:** Continuous audio feedback.
  - **SQUELCH:** Audio output enabled only if the signal strength exceeds the sensitivity threshold.

## 📸 Screenshots

![screenshot_scanner](screenshots/screenshot_scanner.png)

![screenshot_config](screenshots/screenshot_config.png)


## 🤔 ToDo

- Currently using FM238 (FuriHalSubGhzPreset2FSKDev238Async) but per CodeAllNight a custom preset can be used to improve audio.

![rocketgod_logo](https://github.com/RocketGod-git/shodanbot/assets/57732082/7929b554-0fba-4c2b-b22d-6772d23c4a18)
