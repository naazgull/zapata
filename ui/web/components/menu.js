export default {
    props: {
        config: Object,
        items: Array
    },
    methods: {
        change_language(language) {
            this.config.set_lang(language)
            window.location.reload(true)
        }
    },
    data() {
        return {}
    },
    template: `
    <div>
      <div v-for="item in items">
        <a :href="item.url">{{ item.label[config.get_lang()] }}</a>
      </div>
    </div>
    <div>
      <div v-for="(value, item) in config.languages">
        <a @click="change_language(item)">{{ value }}</a>
      </div>
    </div>
  `
}
