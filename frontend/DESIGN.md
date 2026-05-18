---
name: Cyber-Tech Analytics
colors:
  surface: '#0c1324'
  surface-dim: '#0c1324'
  surface-bright: '#33394c'
  surface-container-lowest: '#070d1f'
  surface-container-low: '#151b2d'
  surface-container: '#191f31'
  surface-container-high: '#23293c'
  surface-container-highest: '#2e3447'
  on-surface: '#dce1fb'
  on-surface-variant: '#cbc3d7'
  inverse-surface: '#dce1fb'
  inverse-on-surface: '#2a3043'
  outline: '#958ea0'
  outline-variant: '#494454'
  surface-tint: '#d0bcff'
  primary: '#d0bcff'
  on-primary: '#3c0091'
  primary-container: '#a078ff'
  on-primary-container: '#340080'
  inverse-primary: '#6d3bd7'
  secondary: '#5de6ff'
  on-secondary: '#00363e'
  secondary-container: '#00cbe6'
  on-secondary-container: '#00515d'
  tertiary: '#ffb2b7'
  on-tertiary: '#67001b'
  tertiary-container: '#ff516a'
  on-tertiary-container: '#5b0017'
  error: '#ffb4ab'
  on-error: '#690005'
  error-container: '#93000a'
  on-error-container: '#ffdad6'
  primary-fixed: '#e9ddff'
  primary-fixed-dim: '#d0bcff'
  on-primary-fixed: '#23005c'
  on-primary-fixed-variant: '#5516be'
  secondary-fixed: '#a2eeff'
  secondary-fixed-dim: '#2fd9f4'
  on-secondary-fixed: '#001f25'
  on-secondary-fixed-variant: '#004e5a'
  tertiary-fixed: '#ffdadb'
  tertiary-fixed-dim: '#ffb2b7'
  on-tertiary-fixed: '#40000d'
  on-tertiary-fixed-variant: '#92002a'
  background: '#0c1324'
  on-background: '#dce1fb'
  surface-variant: '#2e3447'
typography:
  headline-lg:
    fontFamily: Geist
    fontSize: 32px
    fontWeight: '600'
    lineHeight: 40px
    letterSpacing: -0.02em
  headline-md:
    fontFamily: Geist
    fontSize: 24px
    fontWeight: '600'
    lineHeight: 32px
    letterSpacing: -0.01em
  body-md:
    fontFamily: Geist
    fontSize: 16px
    fontWeight: '400'
    lineHeight: 24px
    letterSpacing: 0em
  body-sm:
    fontFamily: Geist
    fontSize: 14px
    fontWeight: '400'
    lineHeight: 20px
    letterSpacing: 0em
  label-mono:
    fontFamily: JetBrains Mono
    fontSize: 13px
    fontWeight: '500'
    lineHeight: 16px
    letterSpacing: 0.02em
  code-sm:
    fontFamily: JetBrains Mono
    fontSize: 12px
    fontWeight: '400'
    lineHeight: 16px
    letterSpacing: 0em
rounded:
  sm: 0.125rem
  DEFAULT: 0.25rem
  md: 0.375rem
  lg: 0.5rem
  xl: 0.75rem
  full: 9999px
spacing:
  unit: 4px
  gutter: 16px
  margin-desktop: 24px
  margin-mobile: 12px
  panel-padding: 12px
---

## Brand & Style
The design system is engineered for high-density technical analysis and complex graph visualization. It adopts a **Cyber-Tech** aesthetic—a fusion of modern minimalism and high-contrast technical observability. The UI is designed to feel like a high-performance instrument: precise, authoritative, and focused.

The visual narrative prioritizes data visibility against a deep, void-like backdrop, utilizing vibrant neon accents to categorize entities and relationships. This approach ensures that critical data points (nodes) and their connections (links) remain the primary focus, even within high-volume datasets. The emotional response is one of total control and clarity amidst complexity.

## Colors
The palette is centered on a "Deep Space" foundation to maximize the luminosity of technical data.

- **Background (#020617):** The primary canvas, providing maximum contrast for neon elements.
- **Electric Violet (#8b5cf6):** Reserved for User nodes, primary system actors, and core entities.
- **Neon Cyan (#22d3ee):** Used for interaction links, data flow lines, and successful system states.
- **Alert Red (#f43f5e):** Strictly for anomalies, critical errors, and high-risk technical scores.
- **Neutral Grays:** A range of slates (from #1e293b to #94a3b8) are used for UI chrome, borders, and secondary metadata to prevent visual fatigue.

All interactive elements must maintain a high contrast ratio against the dark background to ensure readability of technical IDs and scores.

## Typography
Typography is split between **Geist** for structural UI and **JetBrains Mono** for data-heavy strings.

- **Geist (Sans):** Used for navigation, headers, and general interface text. It provides a clean, modern, and neutral framework that doesn't distract from the data.
- **JetBrains Mono (Monospace):** Critical for technical strings, IDs, timestamps, and scores. The monospaced nature ensures that columns of numbers align perfectly, aiding in rapid scanning and pattern recognition.

For mobile viewports, `headline-lg` should scale down to 24px to maintain layout integrity while preserving the high-contrast readability of the monospace labels.

## Layout & Spacing
The layout follows a **Fixed-Fluid Hybrid** model typical of dashboard environments. 

- **Sidebar/Control Panels:** Fixed width (280px) to house dense filtering and query tools.
- **Main Canvas:** Fluid area for the graph visualization.
- **Spacing Rhythm:** Based on a 4px baseline grid. Components use tight padding (8px or 12px) to maximize the information density ("Data-to-Ink ratio").

On mobile devices, sidebars collapse into bottom sheets or full-screen overlays. The graph canvas remains fluid but allows for pinch-to-zoom interaction.

## Elevation & Depth
In this dark, high-contrast environment, depth is achieved through **Tonal Layers** and **Subtle Glows** rather than traditional shadows.

- **Base Layer (#020617):** The main application background.
- **Surface Layer (#0f172a):** Used for cards, panels, and sidebars. 1px borders (#1e293b) define edges.
- **Elevated Layer (#1e293b):** Used for tooltips and hover states.
- **Accent Glow:** Active nodes or selected links utilize a soft `0px 0px 8px` outer glow in their respective accent color (Violet or Cyan) to signify focus without adding bulk to the element.

## Shapes
The shape language is "Technical Soft"—precise but accessible. 

A uniform **4px (0.25rem)** border radius is applied to buttons, input fields, and panels. This keeps the UI feeling structured and modular. Circular shapes are reserved exclusively for User Nodes within the graph to distinguish them from rectangular data blocks or control elements.

## Components
- **Buttons:** Primary buttons use a solid Electric Violet fill with white text. Secondary buttons use a Cyan outline with no fill.
- **Chips/Badges:** Small, monospaced text badges for "Technical Scores." Use high-contrast backgrounds: a dark cyan tint for positive scores and a dark red tint for anomalies.
- **Input Fields:** Dark slate background (#0f172a) with a 1px border. On focus, the border transitions to Neon Cyan with a subtle outer glow.
- **Graph Nodes:** Circular for users (Electric Violet), square for system assets (Slate). Active states feature a 2px Neon Cyan ring.
- **Interaction Links:** 1.5px weighted lines. Inactive links are muted slate; active/hovered links glow in Neon Cyan.
- **Data Tables:** Dense layout with JetBrains Mono text. Row hovering should use a subtle highlight (#1e293b) to assist horizontal tracking.