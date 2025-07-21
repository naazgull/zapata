export default {
    props: {
        listen: String
    },
    data() {
        let id = window.location.hash.substring(2)
        let show = id ? true : false
        this.register_listener()
        return {
            show: show
        }
    },
    methods: {
        register_listener() {
            switch(this.listen) {
            case 'hashtag':{
                window.addEventListener('hashchange', () => {
                    let id = window.location.hash.substring(2)
                    this.show = id ? true : false
		            })
                break;
            }
            }
        },
        close() {
            switch(this.listen) {
            case 'hashtag':{
                window.location.hash = ''
                break
            }
            default: {
                this.show = false
            }
            }
        }
    },
    template: `
    <transition name="modal">
      <div v-if="show" class="modal-mask">
        <div class="modal-container">
          <button
            class="modal-default-button"
            @click="close()">x</button>
          <slot name="content"></slot>
        </div>
      </div>
    </transition>
  `
}
