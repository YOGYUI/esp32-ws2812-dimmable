import Vue from 'vue'
import VueRouter from 'vue-router'

Vue.use(VueRouter)

const routes = [
  {
    path: '/',
    name: 'home',
    component: () => import('../views/HomeView.vue')
  },
  {
    path: '/ws2812',
    name: 'ws2812',
    component: () => import('../views/WS2812View.vue')
  },
  {
    path: '/dpot',
    name: 'dpot',
    component: () => import('../views/DPOTView.vue')
  }
]

const router = new VueRouter({
  mode: 'history',
  base: process.env.BASE_URL,
  routes
})

export default router
