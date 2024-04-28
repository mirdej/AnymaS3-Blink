<script setup>
import axios from 'axios';
import Sidebar from 'primevue/sidebar';
import Knob from 'primevue/knob';
import { ref, computed, onMounted, watch } from 'vue'


//-------------------------------------------------------  REACTIVE
const deviceInfo_visible = ref(false);
const device = ref({ "message": "deviceinfo", "firmware": "unknown", "version": "unknown", "chip": "unknown", "chip_rev": 0, "chip_cores": 0, "chip_fcpu": 0, "sdk_version": "unknown", "flash_size": 0, "flash_speed": 0, "flash_mode": 0, "flash_free_sketch_space": 0, "hostname": "unknown", "ip": "unknown", "mac": "unknown", "millis": 0, "ram_free": 0, "ram_lowest": 0, "ram_largest_free_block": 0, "psram_size": 0, "psram_free": 0, "psram_free_min": 0, "psram_max_alloc": 0, "fs_used": 0, "fs_total": 1 });



const percentFreeRam = computed(() => {
    var f = device.value.ram_free / 327680. * 100.;
    return parseInt(f)
})

const percentFreeFS = computed(() => {
    var f = device.value.fs_used / device.value.fs_total * 100.;
    return 100 - parseInt(f)
})

var intervalHandle;
watch(deviceInfo_visible, (vis) => {/* console.log(vis) */
    if (vis) {
        updateDeviceInfo();
        intervalHandle = setInterval(updateDeviceInfo, 2000)
    } else {
        clearInterval(intervalHandle)
    }
})


//-------------------------------------------------------  GLOBALS
var host = "baby-s3.local"
var ip = location.host;
if (ip.startsWith('127') || ip.startsWith('localhost')) {
    host = "http://baby-s3.local/";
} else {
    host = "http://" + ip + "/";
}


//-------------------------------------------------------  INTERACTION

const onDeviceButtonClick = () => {
    deviceInfo_visible.value = !deviceInfo_visible.value;
}


//-------------------------------------------------------  UTILITY / DISPLAY

function humanFileSize(size) {
    var i = size == 0 ? 0 : Math.floor(Math.log(size) / Math.log(1024));
    return +((size / Math.pow(1024, i)).toFixed(2)) * 1 + ' ' + ['B', 'kB', 'MB', 'GB', 'TB'][i];
}

function capitalizeFirstLetter(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
}

const compiletime = (t) => {
    var timestamp = t.split('+')[1];
    var thedate = new Date(timestamp * 1000);
    return thedate.toLocaleString();
}

const uptime = (m) => {
    return new Date(m).toISOString().slice(11, -5);
}

const updateDeviceInfo = () => {
    if (deviceInfo_visible) {
        axios.get(host + 'api/deviceinfo', { timeout: 2000 })

            .then(function (response) {
                device.value = response.data;
                console.log(response);
            })
            .catch(function (error) {
                console.log(error);
                toast.add({ severity: 'danger', summary: 'An error occured', detail: error });
            })
            .finally(function () {
                // always executed
            });
    }
}

</script>



$


<template>
    <button class="device-button p-link" type="button" @click="onDeviceButtonClick()">
        <i class="pi pi-info-circle" style="font-size:1.8em;color:#777"></i>
    </button>
    <Sidebar class="device_info" v-model:visible="deviceInfo_visible" header="Device Info" position="bottom"
        style="height:auto; color:#888">
        <!-- <h1>Testing  {{ device.chip }}</h1> -->
        <p><span class="device_info_bold">Chip: </span> {{ device.chip }} | Cores: {{ device.chip_cores }} | {{
            device.chip_fcpu }} Mhz | SDK: {{ device.sdk_version }}</p>
        <p><span class="device_info_bold">Firmware: </span>{{ device.firmware }} | Version: {{ device.version }} | {{
            compiletime(device.version) }}</p>
        <p><span class="device_info_bold">RAM: </span> Free: {{ humanFileSize(device.ram_free) }} | Largest free block :
            {{ humanFileSize(device.ram_largest_free_block) }} | Lowest: {{ humanFileSize(device.ram_lowest) }}</p>
        <Knob :size="64" v-model="percentFreeRam" valueTemplate="{value}%" />

        <p><span class="device_info_bold">Flash: </span>{{ humanFileSize(device.flash_size) }} | Free Sketch Size:
            {{ humanFileSize(device.flash_free_sketch_space) }}</p>
        <p><span class="device_info_bold">Uptime: </span> {{ uptime(device.millis) }}</p>
        <p><span class="device_info_bold">Hostname:</span> {{ capitalizeFirstLetter(device.hostname) }} | IP: {{
            device.ip }} | MAC: {{ device.mac }}</p>
        <p><span class="device_info_bold">Filesystem:</span>{{ humanFileSize(device.fs_used) }} of {{
            humanFileSize(device.fs_total) }} ({{ parseInt(device.fs_used / device.fs_total * 100) }} %)</p>
        <Knob :size="64" v-model="percentFreeFS" valueTemplate="{value}%" />

    </Sidebar>

</template>






<style scoped>
.device_info p {
    font-size: .8em;
}

.device_info_bold {
    font-weight: bold;
    color:
        #335d5a;
}

.device-button {
    position: absolute;
    bottom: 6px;
    right: 14px
}
</style>