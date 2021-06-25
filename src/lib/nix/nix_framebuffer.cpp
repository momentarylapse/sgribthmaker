/*
 * nix_framebuffer.cpp
 *
 *  Created on: Jun 25, 2021
 *      Author: michi
 */

#if HAS_LIB_GL


#include "nix.h"
#include "nix_common.h"



namespace nix {

FrameBuffer *cur_framebuffer = nullptr;


FrameBuffer *FrameBuffer::DEFAULT = new FrameBuffer();

FrameBuffer::FrameBuffer() {
	depth_buffer = nullptr;
	width = height = 0;

	frame_buffer = 0;
	multi_samples = 0;
}

FrameBuffer::FrameBuffer(const Array<Texture*> &attachments) {
	glCreateFramebuffers(1, &frame_buffer);
	update(attachments);
}

FrameBuffer::~FrameBuffer() {
	glDeleteFramebuffers(1, &frame_buffer);
}

void FrameBuffer::__init__(const Array<Texture*> &attachments) {
	new(this) FrameBuffer(attachments);
}

void FrameBuffer::__delete__() {
	this->~FrameBuffer();
}

void FrameBuffer::update(const Array<Texture*> &attachments) {
	update_x(attachments, -1);
}

void FrameBuffer::update_x(const Array<Texture*> &attachments, int cube_face) {
	depth_buffer = nullptr;
	color_attachments = {};
	int samples = 0;

	for (auto *a: attachments) {
		if ((a->type == a->Type::DEPTH) or (a->type == a->Type::RENDERBUFFER))
			depth_buffer = (DepthBuffer*)a;
		else
			color_attachments.add(a);
		if (a->width > 0) {
			width = a->width;
			height = a->height;
		}
		if (a->samples > 0)
			samples = a->samples;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);



	if (depth_buffer) {
		if (depth_buffer->type == Texture::Type::RENDERBUFFER) {
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer->texture);
		} else {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_buffer->texture, 0);
		}
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	Array<GLenum> draw_buffers;
	int target =  GL_TEXTURE_2D;
	if (cube_face >= 0)
		target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + cube_face;
	if (samples > 0)
		target = GL_TEXTURE_2D_MULTISAMPLE;
	foreachi (auto *t, color_attachments, i) {
		//glNamedFramebufferTexture(frame_buffer, GL_COLOR_ATTACHMENT0 + i, t->texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, t->texture, 0);
		draw_buffers.add(GL_COLOR_ATTACHMENT0 + (unsigned)i);
	}
	glDrawBuffers(draw_buffers.num, &draw_buffers[0]);


	_check();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::_check() {
	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		msg_error("FrameBuffer: framebuffer != complete");
		if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
			msg_write("incomplete att");
		//if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
		//	msg_write("incomplete dim");
		if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
			msg_write("missing att");
		if (status == GL_FRAMEBUFFER_UNSUPPORTED)
			msg_write("unsup");
	}
}

rect FrameBuffer::area() const {
	return rect(0, width, 0, height);
}

void FrameBuffer::clear_color(int index, const color &c) {
	glClearNamedFramebufferfv(frame_buffer, GL_COLOR, index, (float*)&c);
}

void FrameBuffer::clear_depth(float depth) {
	glClearNamedFramebufferfv(frame_buffer, GL_DEPTH, 0, &depth);
}

void bind_frame_buffer(FrameBuffer *fb) {
	glBindFramebuffer(GL_FRAMEBUFFER, fb->frame_buffer);
	cur_framebuffer = fb;

	set_viewport(fb->area());
}

void resolve_multisampling(FrameBuffer *target, FrameBuffer *source) {
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->frame_buffer);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, source->frame_buffer);
	//glBlitFramebuffer(0, 0, source->width, source->height, 0, 0, target->width, target->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBlitNamedFramebuffer(source->frame_buffer, target->frame_buffer, 0, 0, source->width, source->height, 0, 0, target->width, target->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

}

#endif

