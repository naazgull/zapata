export default {
    props: {
        id: String,
        config: Object
    },
    data() {
        return {
            show: false
        }
    },
    computed: {
        input_data() {
            let show = this.show
            let config = this.config
        }
    },
    template: `
    <label :id="id + '_label'" :for="id">{{ config.label }}</label>
    
  `
}
