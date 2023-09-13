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
    </div>
  `
}
