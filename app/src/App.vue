<script setup>
import { ref } from 'vue'
import { useToast } from 'primevue/usetoast';

const blink_time = ref();
const toast = useToast();

const setblink = () =>{
toast.add({severity:'success',summary:'Blink time updated',detail:"Time: "+blink_time.value+" ms", life: 2000});
}
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
