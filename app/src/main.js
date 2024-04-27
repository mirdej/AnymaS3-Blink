import { createApp } from 'vue'
import ToastService from 'primevue/toastservice';


import App from "./App.vue"
import PrimeVue from 'primevue/config';

import Button from 'primevue/button';
import InputText from 'primevue/inputtext';
import InputNumber from 'primevue/inputnumber';
import Toast from 'primevue/toast';
import FloatLabel from 'primevue/floatlabel';

import './assets/app.css'
import 'primevue/resources/themes/aura-dark-teal/theme.css'
import 'primeicons/primeicons.css'

const app = createApp(App);
app.use(PrimeVue);
app.use(ToastService);

app.component("Button",Button);
app.component("InputText",InputText);
app.component("InputNumber",InputNumber);
app.component("Toast",Toast);
app.component("FloatLabel",FloatLabel);

app.mount("#app")
