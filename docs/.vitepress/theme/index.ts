// https://vitepress.dev/guide/custom-theme
import { h } from 'vue'
import Theme from 'vitepress/theme'
import './style.css'

import HomeContent from './HomeContent.vue'

export default {
  ...Theme,
  Layout: () => {
    return h(Theme.Layout, null, {
      'home-features-after': () => h(HomeContent)
      // https://vitepress.dev/guide/extending-default-theme#layout-slots
    })
  },
  enhanceApp({ app, router, siteData }) {
    // ...
  }
}
