const { defineConfig } = require('@vue/cli-service')
module.exports = defineConfig({
  transpileDependencies: [
    'vuetify'
  ],
  lintOnSave: false,  // prevent "multi-word-component" error,
  devServer: {
    allowedHosts: 'all' // prevent 'Invalid Host header'
  },
})
