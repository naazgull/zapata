export default {
    props: {
        lang: String,
        dictionary: Object,
        id: String,
        target_url: String,
        fields: Object,
        clear_on_save: Boolean
    },
    data() {
        let inputs = {}
        let document = {}
        for (let key of Object.keys(this.fields)) {
            let field = this.fields[key]
            field.name = key
            field.id = this.id + '_document_' + key
        }
        return {
            document_id: null,
            fetched: {},
            messages: { result: null, fields: {} },
            received: false,
            show: 0,
            submission_url: this.target_url,
            submission_method: 'POST'
        }
    },
    computed: {
        document() {
            let show = this.show
            let document_id = this.document_id

            if (!document_id) {
                this.show = 100
                this.submission_url = this.target_url.replaceAll('{id}', '')
                this.submission_method = 'POST'
                return this.initialize_document()
            }

            this.submission_url = this.target_url.replaceAll('{id}', document_id)
            
            if (this.received) {
                this.show = 100
                this.received = false
                return this.fetched
            }

            if (document_id) {
                let url = this.submission_url
                url += '?fields=' + Object.keys(this.fields).join(",")

                this.fetch_data(url).then((data) => {
                    this.received = true
                    this.fetched = data
                })
                this.submission_method = 'PATCH'
                return this.fetched
            }
        }
    },
    watch: {
        document: {
            handler() {
                this.validate_data(false)
            },
            deep: true
        }
    },
    methods: {
        initialize_document() {
            let document = {}
            for (let key of Object.keys(this.fields)) {
                document[key] = null
            }
            return document
        },
        listen_to_hash() {
            let item = window.location.hash.substring(2)
            if (item) {
                if (item == '?action=new') {
                }
                else {
                    this.document_id = item
                }
            }
            else {
                this.document_id = null
            }
        },
        async fetch_data(url) {
            try {
                const reply = await fetch(url)
                if (reply.status == 200) {
                    const data = await reply.json()
                    return data
                }
            } catch (error) {
                console.log(error)
            }
            return {}
        },
        validate_data(with_required) {
            let valid = true
            for (let key of Object.keys(this.document)) {
                let has_message = false

                if (!this.fields[key]) {
                    continue
                }
                
                if (with_required && !this.document[key] && this.fields[key].required) {
                    this.messages.fields[key] = this.dictionary.required_message[this.lang]
                    has_message = true
                    valid = false
                }
                if (this.document[key] && this.fields[key].format) {
                    const regexp = new RegExp(this.fields[key].format)
                    if (!regexp.test(this.document[key])) {
                        this.messages.fields[key] = (this.fields[key].help ?
                                                     this.fields[key].help : '')
                        has_message = true
                        valid = false
                    }
                }

                if (!has_message) {
                    delete this.messages.fields[key]
                }
            }
            return valid
        },
        async send_data(event) {
            event.preventDefault();
            if (this.validate_data(true)) {
                const req = {
                    method: this.submission_method,
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(this.document)
                }
                const reply = await fetch(this.submission_url, req)
                const data = await reply.json()
                if (reply.status > 299) {
                    this.messages.result_type = 'remote-error'
                    this.messages.result = this.dictionary.request_error[this.lang] + ': '
                        + data.what
                }
                else if (this.clear_on_save) {
                    window.location.hash = ''
                }
                else {
                    this.messages.result_type = 'remote-success'
                    this.messages.result = this.dictionary.request_success[this.lang]
                }
            }
        },
    },
    created() {
        window.addEventListener('hashchange', () => {
            this.listen_to_hash()
		    })
        this.listen_to_hash()
    },
    template: `
    <div :id="id" :style="{ opacity: show }" style="transition: opacity 0.5s ease">
      <p v-if="messages.result" :class="messages.type">{{ messages.result }}</p>
      <form :id="id + '_document'" @submit.prevent="send_data">
        <div v-for="key in Object.keys(fields)">
          <label :for="fields[key].id">{{ fields[key].label }}</label>
          <div v-if="!fields[key].multiple">
            <input v-model="document[key]"
              :type="fields[key].type"
              :id="fields[key].id"
              :name="fields[key].name"
              :placeholder="fields[key].help">
            <span v-if="messages.fields[key]" class="field-error">{{ messages.fields[key] }}</span>
          </div>
        </div>
        <input type="submit" :value="dictionary.button_label[lang]">
      </form>
    </div>
  `
}
