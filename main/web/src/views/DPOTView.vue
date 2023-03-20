<template>
    <v-container>
        <v-row
            class="text-center"
        >
            <v-col
                class="mb-1"
            >
                <h3>Raw Value</h3>
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
                    v-model="dpot_raw_value"
                    :max="255"
                    :min="0"
                    class="align-center"
                    @change="handle_raw_value_slider"
                >
                    <template v-slot:append>
                        <v-text-field
                            v-model="dpot_raw_value"
                            type="number"
                            class="mt-0 pt-0"
                            style="width: 50px"
                            @change="handle_raw_value_slider"
                        >
                        </v-text-field>
                    </template>
                </v-slider>
            </v-col>
        </v-row>
    </v-container>
</template>

<script>
export default {
    name: 'DPOTView',
    data() {
        return {
            dpot_raw_value: 0
        }
    },
    beforeMount: function() {
        this.get_dpot_state();
    },
    methods: {
        handle_raw_value_slider: function(value) {
            this.set_dpot_raw_value(value);
        },
        get_dpot_state: function() {
            this.$axios({
                method: "get",
                url: "/api/v1/dpot/state"
            })
            .then(response => {
                console.log(response.data);
                this.dpot_raw_value = response.data.raw_value;
            })
            .catch(error => {
                console.log(error);
            })
        },
        set_dpot_raw_value: function(value) {
            this.$axios({
                method: "post",
                url: "/api/v1/dpot/config",
                data: JSON.stringify({
                    raw_value: value
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