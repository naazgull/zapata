export default {
    props: {
        trigger: String,
        label: String,
        event: String
    },
    data() {
        return {}
    },
    methods: {
        trigger_event() {
            window.location.hash = '/' + this.event
        }
    },
    template: `
    <div class="new-record">
      <button @click="trigger_event()">{{ label }}</button>
    </div>
  `
}
