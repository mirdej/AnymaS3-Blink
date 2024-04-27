<script setup>
import { ErrorCodes, ref } from 'vue'
import { useToast } from 'primevue/usetoast';
import { onMounted } from 'vue'
import axios from 'axios';

var host;


const blink_time = ref();
const toast = useToast();

const setblink = () => {
  /*  axios
     .get('http://baby-s3.local/blink_interval?set=' + blink_time.value)
     .then(response => (){ blink_time.value = response.data; toast.add({ severity: 'success', summary: 'Blink time updated', detail: "Time: " + blink_time.value + " ms", life: 2000 }})
 
 ); */

  axios.get(host+'api/blink_interval', {
    params: {
      set: blink_time.value
    },timeout:2000
  })
  
    .then(function (response) {
      console.log(response);
      toast.add({ severity: 'success', summary: 'Blink time updated', detail: "Time: " + blink_time.value + " ms", life: 2000 });
    })
    .catch(function (error) {
      console.log(error);
      toast.add({ severity: 'danger', summary: 'An error occured', detail:error });
    })
    .finally(function () {
      // always executed
    });
}


onMounted(() => {

  var ip = location.host;
            if (ip.startsWith('127')||ip.startsWith('localhost')) {
                host = "http://baby-s3.local/";
            } else {
                host = "http://" + ip + "/";
            }


  console.log(host)

  axios
    .get(host+'api/blink_interval')
    .then(response => (blink_time.value = response.data))
})

</script>

<template>
  <div class="container">
    <!--     <h1>Hello Michael</h1 -->
    <FloatLabel>
      <InputNumber id="blink_time" suffix=" ms" v-model="blink_time" />
      <label for="blink_time">Blink time</label>
    </FloatLabel>
    <Button label="Set" @click="setblink" icon="pi pi-arrow-circle-up"></Button>
  </div>
  <Toast />
</template>

<style scoped>
/* "that's probably the only CSS you'll need to know about"  Cagatay Civici, PrimeVue */
.container {
  display: flex;
  align-items: center;
  justify-content: center;
  min-height: 100vh;
}
</style>
