import { computed } from 'vue'

export default {
    props: {
        lang: String,
        dictionary: Object,
        id: String,
        target_url: String,
        fields: Object,
        visible: Array,
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
                url += '?fields=' + this.visible.join(",")

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
    provide() {
        return {
            document: computed(() => this.document),
            fields: this.fields
        }
    },
    methods: {
        initialize_document() {
            let url_parameters = this.get_url_parameters()
            let document = {}
            for (let key of this.visible) {
                if (this.fields[key].multiple) {
                    document[key] = []
                }
                else if (url_parameters[key]) {
                    document[key] = url_parameters[key]
                }
                else{
                    document[key] = null
                }
            }
            return document
        },
        listen_to_hash() {
            let item = window.location.hash.substring(2)
            if (item) {
                if (item == '?action=new') {
                    this.document_id = null
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
                let req = undefined;

                if (event.submitter.id == this.id + '_ok') {
                    req = {
                        method: this.submission_method,
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify(this.document)
                    }
                }
                else if (event.submitter.id == this.id + '_remove') {
                    req = {
                        method: 'DELETE',
                        headers: { 'Content-Type': 'application/json' }
                    }
                }

                const reply = await fetch(this.submission_url, req)
                const data = await reply.json()
                if (reply.status > 299) {
                    this.messages.result_type = 'remote-error'
                    this.messages.result = this.dictionary.request_error[this.lang] + ': '
                        + data.message
                }
                else if (this.clear_on_save) {
                    window.location.hash = ''
                }
                else {
                    if (data.removed_count) {
                        window.location.hash = '/?action=new'
                    }
                    else {
                        if (data.element_id) {
                            window.location.hash = '/' + data.element_id[0]
                        }
                        this.messages.result_type = 'remote-success'
                        this.messages.result = this.dictionary.request_success[this.lang]
                    }
                }
            }
        },
        get_url_parameters() {
            let uri = window.location.href.split('?');
            if(uri.length > 1) {
                let result = {}
                let elements = uri[1].split('#')[0].split('&')
                for (let element of elements) {
                    let kv = element.split('=')
                    result[kv[0]] = kv[1]
                }
                return result
            }
            return {}
        },
        generate_link(value) {
            let link = value.replaceAll('{id}', this.document_id)
            for (let key of Object.keys(this.document)) {
                link = link.replaceAll('{' + key + '}', this.document[key])
            }
            return link
        }
    },
    created() {
        window.addEventListener('hashchange', () => {
            this.listen_to_hash()
		    })
        this.listen_to_hash()
    },
    template: `
    <div :id="id"
      class="zpt-form" :class="id.replace('_', '-')"
      :style="{ opacity: show }" style="transition: opacity 0.5s ease">
      <p v-if="messages.result"
        class="zpt-form-message" :class="messages.type">
        {{ messages.result }}
      </p>
      <form :id="id + '_document'" @submit.prevent="send_data">
        <div v-for="key in visible"
          class="zpt-input-container" :class="'zpt-' + fields[key].type.replace('zpt:', '') + '-container'">
          <label v-if="[ 'link', 'separator' ].indexOf(fields[key].type) == -1 && fields[key].type.indexOf('zpt:') == -1"
            :for="fields[key].id">{{ fields[key].label }}</label>
          <div v-if="[ 'radio', 'dropbox', 'checkbox', 'link', 'separator', 'static' ].indexOf(fields[key].type) == -1 && fields[key].type.indexOf('zpt:') == -1">
            <input v-model="document[key]"
              :type="fields[key].type"
              :id="fields[key].id"
              :name="fields[key].name"
              :placeholder="fields[key].help">
            <span v-if="messages.fields[key]" class="field-error">{{ messages.fields[key] }}</span>
          </div>
          <div v-if="fields[key].type == 'dropbox'">
            <select v-model="document[key]"
              :id="fields[key].id"
              :name="fields[key].name"
              :placeholder="fields[key].help">
              <option v-for="op in fields[key].options"
                :key="op.value"
                :value="op.value">
                {{ op.text }}
              </option>
            </select>
          </div>
          <div v-if="fields[key].type == 'checkbox'">
            <div v-for="(op, idx) in fields[key].options">
              <input v-model="document[key]"
                :type="fields[key].type"
                :id="fields[key].id + '_' + idx"
                :value="op.value">
              <label :for="fields[key].id + '_' + idx">{{ op.text }}</label>
            </div>
            <span v-if="messages.fields[key]" class="field-error">{{ messages.fields[key] }}</span>
          </div>
          <div v-if="fields[key].type == 'radio'">
            <div v-for="(op, idx) in fields[key].options">
              <input v-model="document[key]"
                :name="fields[key].name"
                :type="fields[key].type"
                :id="fields[key].id + '_' + idx"
                :value="op.value">
              <label :for="fields[key].id + '_' + idx">{{ op.text }}</label>
            </div>
            <span v-if="messages.fields[key]" class="field-error">{{ messages.fields[key] }}</span>
          </div>
          <div v-if="fields[key].type == 'static'">
            <span>{{ document[key] }}</span>
          </div>
          <div v-if="fields[key].type == 'link' && document_id">
            <a :href="generate_link(document[key] ? document[key] : fields[key].value.href)"
               :title="fields[key].help">{{ fields[key].label }}</a>
          </div>
          <div v-if="fields[key].type == 'separator'">
            <p><hr/>{{ fields[key].label }}</p>
          </div>
        </div>
        <slot name="components"></slot>
        <input type="submit"
          class="zpt-form-ok"
          :id="id + '_ok'"
          :value="dictionary.ok_button_label[lang]">
        <input type="submit"
          class="zpt-form-remove"
          :id="id + '_remove'"
          :value="dictionary.remove_button_label[lang]">
      </form>
    </div>
  `
}
