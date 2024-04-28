<script setup>
import { ErrorCodes, ref, watch } from 'vue'
import { useToast } from 'primevue/usetoast';
import { onMounted } from 'vue'
import axios from 'axios';
import DeviceInfo from "./DeviceInfo.vue"
import AppFrame from "./AppFrame.vue"

var host;

const blink_color = ref();
const blink_interval = ref();
const toast = useToast();


const device_info = ref({ "message": "deviceinfo", "firmware": "unknown", "version": "unknown", "chip": "ESP32-S3", "chip_rev": 0, "chip_cores": 2, "chip_fcpu": 240, "sdk_version": "v4.4.6-dirty", "flash_size": 16777216, "flash_speed": 80000000, "flash_mode": 0, "flash_free_sketch_space": 1310720, "hostname": "baby-s3", "ip": "192.168.252.107", "mac": "F4:12:FA:4F:C7:9C", "millis": 49129, "ram_free": 215204, "ram_lowest": 147168, "ram_largest_free_block": 192500, "psram_size": 0, "psram_free": 0, "psram_free_min": 0, "psram_max_alloc": 0, "fs_used": 1630208, "fs_total": 14024704 });

const setblink = () => {
  console.log(blink_color.value)

  axios.get(host + 'api/blink_interval', {
    params: {
      set: blink_interval.value
    }, timeout: 2000
  })

    .then(function (response) {
      console.log(response);
      toast.add({ severity: 'success', summary: 'Blink time updated', detail: "Time: " + blink_interval.value + " ms", life: 2000 });
    })
    .catch(function (error) {
      console.log(error);
      toast.add({ severity: 'danger', summary: 'An error occured', detail: error });
    })
    .finally(function () {
      // always executed
    });
}

watch(blink_color, (now, before) => {
  console.log(now, before)
  axios.get(host + 'api/blink_color', {
    params: {
      set: now
    }, timeout: 2000
  }).then(function (response) {
    console.log(response);
  })
    .catch(function (error) {
      console.log(error);
      toast.add({ severity: 'danger', summary: 'An error occured', detail: error });
    })
    .finally(function () {
      // always executed
    });
})

onMounted(() => {

  var ip = location.host;
  if (ip.startsWith('127') || ip.startsWith('localhost')) {
    host = "http://baby-s3.local/";
  } else {
    host = "http://" + ip + "/";
  }


  console.log(host)

  axios
    .get(host + 'api/fetchall')
    // .then(response => (blink_interval.value = response.data))
    .then(response => {
      blink_color.value = response.data.blink_color;
      blink_interval.value = response.data.blink_interval;
      console.log(response)
    })

    axios.get(host + 'api/deviceinfo', {timeout: 2000})

    .then(function (response) {
      device_info.value = response.data;
      console.log(response);
    })
    .catch(function (error) {
      console.log(error);
      toast.add({ severity: 'danger', summary: 'An error occured', detail: error });
    })
    .finally(function () {
      // always executed
    });

})

</script>

<template>
  <div class="container">
    <AppFrame />
    <ColorPicker id="color_picker" v-model="blink_color" style="padding-right:1em;" />

    <FloatLabel>
      <InputNumber id="blink_interval" suffix=" ms" v-model="blink_interval" style="padding-right:1em" />
      <label for="blink_interval">Blink time</label>
    </FloatLabel>
    <Button label="Set" @click="setblink" icon="pi pi-arrow-circle-up"></Button>

    <DeviceInfo :device="device_info"/>

  </div>
  <Toast />
</template>

<style scoped>
/* "that's probably the only CSS you'll need to know about"  Cagatay Civici, PrimeVue */
.container {
  display: flex;
  align-items: center;
  justify-content: center;
  min-height: 90vh;
}

.p-colorpicker-preview {
  height: 200px;
}
</style>
