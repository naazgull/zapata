export default {
    props: {
        lang: String,
        dictionary: Object,
        field: String
    },
    data() {
        return {}
    },
    inject: [
        'document',
        'fields'
    ],
    template: `
    <div v-if="document[field]" :id="fields[field].id">
      <div v-for="key in document[field]">
        <label v-if="dictionary"
          :for="fields[key].id">{{ dictionary[key][lang].label }}</label>
      </div>
    </div>
  `
}
