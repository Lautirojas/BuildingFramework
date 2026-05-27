# Placement Framework

A modular, multiplayer-ready placement framework for Unreal Engine 5.

Designed with reusable systems architecture in mind, supporting:

* data-driven placement definitions
* multiplayer-authoritative placement flow
* runtime preview actors
* grid snapping
* surface validation
* rotation support
* reusable ActorComponent integration
* modular placement item architecture

---

## Features

### Placement System

* Runtime object placement
* Grid snapping support
* Surface angle validation
* Ground tag filtering
* Preview rotation controls
* Configurable placement rules per item

### Multiplayer

* Server-authoritative spawning
* Local cosmetic preview actors
* Replicated placed actors
* Multiplayer-safe placement flow

### Architecture

* Data-driven placement definitions
* Reusable placement components
* Modular runtime placement flow
* Framework-agnostic integration
* Owner-independent component design

## Architecture Overview

The framework separates:

- Placement Item Definitions (static placement configuration)
- Placement Items (runtime placement state)
- Placement Component (runtime placement logic)
- Preview Actors (local placement visualization)

This architecture allows placement systems to remain modular, extensible, and multiplayer-ready.

---

## Example Project

The repository includes an example sandbox project demonstrating:

* runtime object placement
* grid snapping
* multiplayer placement synchronization
* local preview actors

---

## Installation

1. Copy the plugin into your project's `Plugins/` folder
2. Regenerate project files
3. Build the project
4. Enable the plugin from the Unreal Editor if required

---

## Status

Stable initial release (v1.0.0).

The framework is actively maintained and may continue evolving with additional systems and improvements.

---

## Supported Engine Version

Unreal Engine 5.7
