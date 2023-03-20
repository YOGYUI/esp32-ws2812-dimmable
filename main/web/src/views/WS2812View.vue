<template>
    <v-container>
        <v-row
            class="text-center"
        >
            <v-col
                class="mb-1"
            >
                <h3>Brightness</h3>
            </v-col>
        </v-row>

        <v-row
            justify="space-around"
        >
            <v-col
                cols="12"
                md="4"
            >
                <v-slider
                    v-model="brightness"
                    :max="100"
                    :min="0"
                    class="align-center"
                    @change="handle_brightness_slider"
                >
                    <template v-slot:append>
                        <v-text-field
                            v-model="brightness"
                            type="number"
                            class="mt-0 pt-0"
                            style="width: 50px"
                            @change="handle_brightness_slider"
                        >
                        </v-text-field>
                    </template>
                </v-slider>
            </v-col>
        </v-row>
        
        <v-row
            class="text-center"
        >
            <v-col
                class="mb-1"
            >
                <h3>Color</h3>
            </v-col>
        </v-row>

        <v-row
            justify="space-around"
        >
            <v-col
                cols="12"
                md="4"
            >
                <v-color-picker 
                    v-model="picker_color"
                    hide-mode-switch
                    @update:color="handle_picker_color_event"
                    canvas-height="200"
                ></v-color-picker>
            </v-col>
        </v-row>

        <v-row
            justify="space-around"
        >
            <v-col
                cols="12"
                md="1"
            >
                <v-btn
                    color="success"
                    @click="set_ws2812_blink"
                >
                    BLINK
                </v-btn>
            </v-col>
        </v-row>
        
    </v-container>
</template>

<script>
export default {
    name: 'WS2812View',
    data() {
        return {
            picker_color: "#FFFFFF",
            brightness: 0,
            blink_duration: 1000,
            blink_count: 1
        }
    },
    beforeMount: function() {
        this.get_ws2812_state();
    },
    methods: {
        handle_picker_color_event: function(color) {
            // console.log(color.rgba.r, color.rgba.g, color.rgba.b);
            this.set_ws2812_color(color.rgba.r, color.rgba.g, color.rgba.b);
        },
        handle_brightness_slider: function(value) {
            // console.log(value);
            this.set_ws2812_brightness(value);
        },  
        get_ws2812_state: function() {
            this.$axios({
                method: "get",
                url: "/api/v1/ws2812/state"
            })
            .then(response => {
                console.log(response.data);
                this.brightness = response.data.brightness;
                this.picker_color = "#";
                this.picker_color += response.data.red.toString(16).toUpperCase();
                this.picker_color += response.data.green.toString(16).toUpperCase();
                this.picker_color += response.data.blue.toString(16).toUpperCase();
            })
            .catch(error => {
                console.log(error);
            })
        },
        set_ws2812_brightness: function(value) {
            this.$axios({
                method: "post",
                url: "/api/v1/ws2812/config",
                data: JSON.stringify({
                    brightness: value
                }),
                headers: { "Content-Type": "application/json; charset=utf-8" },
                timeout: 5000
            })
            .then(response => {
                
            })
            .catch(error => {
                console.log(error);
            })
        },
        set_ws2812_color: function(red, green, blue) {
            this.$axios({
                method: "post",
                url: "/api/v1/ws2812/config",
                data: JSON.stringify({
                    rgb: [red, green, blue]
                }),
                headers: { "Content-Type": "application/json; charset=utf-8" },
                timeout: 5000
            })
            .then(response => {
                
            })
            .catch(error => {
                console.log(error);
            })
        },
        set_ws2812_blink: function() {
            this.$axios({
                method: "post",
                url: "/api/v1/ws2812/blink",
                data: JSON.stringify({
                    duration: this.blink_duration,
                    count: this.blink_count
                }),
                headers: { "Content-Type": "application/json; charset=utf-8" },
                timeout: 5000
            })
            .then(response => {
                
            })
            .catch(error => {
                console.log(error);
            })
        }
    }
}
</script>