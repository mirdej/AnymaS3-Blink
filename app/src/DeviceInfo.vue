<script setup>
import Sidebar from 'primevue/sidebar';

import { ref, computed } from 'vue'
defineProps(['device'])


function humanFileSize(size) {
    var i = size == 0 ? 0 : Math.floor(Math.log(size) / Math.log(1024));
    return +((size / Math.pow(1024, i)).toFixed(2)) * 1 + ' ' + ['B', 'kB', 'MB', 'GB', 'TB'][i];
}

function capitalizeFirstLetter(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
}



const deviceInfo_visible = ref(true);

//const device = ref({ "message": "deviceinfo", "firmware": "unknown", "version": "0.1.0+1714303415", "chip": "ESP32-S3", "chip_rev": 0, "chip_cores": 2, "chip_fcpu": 240, "sdk_version": "v4.4.6-dirty", "flash_size": 16777216, "flash_speed": 80000000, "flash_mode": 0, "flash_free_sketch_space": 1310720, "hostname": "baby-s3", "ip": "192.168.252.107", "mac": "F4:12:FA:4F:C7:9C", "millis": 49129, "ram_free": 215204, "ram_lowest": 147168, "ram_largest_free_block": 192500, "psram_size": 0, "psram_free": 0, "psram_free_min": 0, "psram_max_alloc": 0, "fs_used": 1630208, "fs_total": 14024704 });



const compiletime = (t) => {
    var timestamp = t.split('+')[1];
    var thedate = new Date(timestamp * 1000);
    return thedate.toLocaleString();
}

const uptime = (m) => {
    return new Date(m).toISOString().slice(11, -5);  // "03:25:45.000"
}


</script>



<template>
    <Sidebar class="device_info" v-model:visible="deviceInfo_visible" header="Device Info" position="bottom"
        style="height:auto; color:#888">
        <!-- <h1>Testing  {{ device.chip }}</h1> -->
<p><span class="device_info_bold">Chip: </span> {{ device.chip }} | Cores: {{ device.chip_cores }} | {{
            device.chip_fcpu }} Mhz | SDK: {{ device.sdk_version }}</p>
        <p><span class="device_info_bold">Firmware: </span>{{ device.firmware }} | Version: {{ device.version }} | {{ compiletime(device.version) }}</p>
<p><span class="device_info_bold">Flash: </span>{{ humanFileSize(device.flash_size) }} | Free Sketch Size:
            {{ humanFileSize(device.flash_free_sketch_space) }}</p>
        <p><span class="device_info_bold">Uptime: </span> {{ uptime(device.millis) }}</p>
        <p><span class="device_info_bold">Hostname:</span> {{ capitalizeFirstLetter(device.hostname) }} | IP: {{
            device.ip }} | MAC: {{ device.mac }}</p>
        <p><span class="device_info_bold">Filesystem:</span>{{ humanFileSize(device.fs_used) }} of {{
            humanFileSize(device.fs_total) }} ({{ parseInt(device.fs_used / device.fs_total * 100) }} %)</p> 
    </Sidebar>

</template>

<style scoped>
.device_info p {
    font-size: .8em;
}

.device_info_bold {
    font-weight: bold;
    color:
        #444;
}
</style>