import { defineConfig } from 'vitepress'

// https://vitepress.dev/reference/site-config
export default defineConfig({
  title: "Open Story Teller",
  description: "Make your own device that tells stories",
  lastUpdated: true,
  cleanUrls: true,


  themeConfig: {

    logo: { src: '/logo_hat_only.png', width: 29, height: 24 },

    // https://vitepress.dev/reference/default-theme-config
    nav: [
      { text: 'Home', link: '/' },
      { text: 'Documentation', link: '/intro-getting-started' }
    ],

    search: {
      provider: 'local'
    },

    sidebar: [
      {
        text: 'Introduction',
        collapsed: false,
        items: [
          { text: 'Getting started', link: '/intro-getting-started' }
        ]
      },
      {
        text: 'Bundles assembly guides',
        collapsed: false,
        items: [
          { text: 'Bundles introduction', link: '/guide-intro' },
          { text: 'Dev kit (Raspberry Pico)', link: '/guide-devkit-pico' }
        ]
      },
      {
        text: 'Software player',
        collapsed: false,
        items: [
          { text: 'Player information', link: '/player-intro' }
        ]
      },
      {
        text: 'Story editor',
        collapsed: false,
        items: [
          { text: 'User manual', link: '/editor-intro' },
          { text: 'Development', link: '/editor-dev' },
        ]
      },
      {
        text: 'Developers corner',
        collapsed: false,
        items: [
          { text: 'Source code architecture', link: '/dev-intro' },
        ]
      }
    ],

    footer: {
      message: 'Source code under the MIT License, art under the CC0 License.',
      copyright: 'Copyright © 2020-present Anthony Rabine'
    },

    socialLinks: [
      { icon: 'github', link: 'https://github.com/arabine/open-story-teller' }
    ]
  },
  locales: {
    root: {
      label: 'English',
      lang: 'en'
    },
    fr: {
      label: 'French',
      lang: 'fr', // optional, will be added  as `lang` attribute on `html` tag
      link: '/fr' // default /fr/ -- shows on navbar translations menu, can be external

      // other locale specific properties...
    }
  }
})

