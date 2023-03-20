<template>
  <v-app>
    <v-app-bar
      app
      color="primary"
      dark
      dense
    >
      <!--
      <div class="d-flex align-center">
        <v-img
          alt="Vuetify Logo"
          class="shrink mr-2"
          contain
          src="https://cdn.vuetifyjs.com/images/logos/vuetify-logo-dark.png"
          transition="scale-transition"
          width="40"
        />

        <v-img
          alt="Vuetify Name"
          class="shrink mt-1 hidden-sm-and-down"
          contain
          min-width="100"
          src="https://cdn.vuetifyjs.com/images/logos/vuetify-name-dark.png"
          width="100"
        />
      </div>
      -->

      <v-btn
        href="./ws2812"
        text
      >
        <span class="mr-2">WS2812</span>
      </v-btn>
      <v-btn
        href="./dpot"
        text
      >
        <span class="mr-2">DPOT</span>
      </v-btn>
      <v-spacer></v-spacer>
    </v-app-bar>

    <v-main>
      <router-view/>
    </v-main>

    <FooterView/>
  </v-app>
</template>

<script>
import FooterView from './views/FooterView.vue';

export default {
  name: 'App',

  components: {
    FooterView
  },

  data: () => ({
    //
  }),
};
</script>
